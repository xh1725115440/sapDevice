// ESP8266设备控制文件
// MADE 2018/7/1
// 两个功能，一个是检测， 一个是配置
// 由于USB串口配置为阻塞模式，我们采用select方式监听超时
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <stdint.h>
#include "usbctl.h"
#include "esp8266.h"
#include <string.h>
#include <stdint.h>

#define DEBUG       //调试宏
#define ToString(x) #x
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define AT #AT
#define AT_RST #AT_RST
#define AT_CWMODE(mode) ToString(AT + CWMODE = ##mode##\r\n)
#define SERVERPORT 8080

// 选择使用的的协议
#define USE_PROTOCOL USE_TCP
#define USE_UDP 0 // 标识使用 0 UDP协议 1 TCP协议
#define USE_TCP 1

#define DELAY_SEC 1

// 遍历com端口，寻找esp8266设备并打开
// 采用 select 方式检测超时
// 先发送 AT 指令测试，测试是否启动，再发送 AT+RST 指令进行重启操作，重启成功，完成配对
// @param
// @return 找到并返回文件描述符，若未找到，返回-1
int esp8266_open()
{
    int fd, ret;
    uint8_t *p;
    int comport = 0;
    uint8_t buf[1024];

    // select 方式
    struct timeval timeout;
    fd_set fdset;
    FD_ZERO(&fdset);

    // 遍历com口
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    fd = open_port(1);
    if (-1 == fd)
    {
#ifdef DEBUG 
        printf("ERROR:Initaled ESP8266 Fail\n");
#endif 
        return -1;
    }
    // 配置USB端口
    ret = set_opt(fd, 115200, 8, 'n', 1);
    FD_SET(fd, &fdset);
    // 开始测试 esp8266设备 AT指令
    // 检测设备启动
    //printf("Start to test the esp8266\n");
    // write_port(fd, "+++", 3);
    // sleep(DELAY_SEC);
    write_port(fd, "AT\r\n", 4);
    sleep(DELAY_SEC);
    // 监听事件 1秒钟 超时跳过
    ret = select(fd + 1, &fdset, NULL, NULL, &timeout);
    switch (ret)
    {
    case -1:
        printf("->> Error occurs when select() \n");
        break;
    case 0:
        printf("->> Select() timeout \n");
        break;
    default:
        if (FD_ISSET(fd, &fdset))
        {
            ret = read_port(fd, buf, 300);
            // printf("1: %s\n", buf);
            if (ret > 0)
            {
                // 检查AT指令返回值
                p = strstr(buf, "OK");
                if (p != NULL)
                {
                    // 重启设备
                    write_port(fd, "AT+RST\r\n", 8);
                    sleep(DELAY_SEC);
                    ret = read(fd, buf, 300);
                    //printf("2: %s\n", buf);
                    // 检查 AT+RST 指令返回值
                    p = strstr(buf, "OK");
                    if (p == NULL)
                    {
                        close(fd);
                        printf("ERROR:ESP8266 RST Failed\n");
                        return -1;
                    }
                    while (read(fd, buf, 1024))
                    {
                        // 检查 AT+RST 指令(重启)是否执行成功
                        //printf("3: %s\n", buf); 
                        p = strstr(buf, "jump");
                        if (p != NULL)
                        {
                            // 完成检测 返回文件描述符
                            tcflush(fd, TCIFLUSH); // 刷新输入缓冲区
                            return fd;
                        }
                    }
                }
            }
        }
    }
    FD_CLR(fd, &fdset);
    close(fd);
    return -1;
}

// esp8266 配置为热点模式
// 设备开启多连接 服务器模式
// 原版本为 TCP 服务器，当前更换为UDP服务器， 丢包问题由上层解决 2018/8/14
// 热点名称 “ESP8266TEST” 密码“12345678”
// @param
// @fd 文件描述符
int esp8266_config(int fd)
{
    //uint8_t TX_buf[8] = {'A', 'T', '+', 'R', 'S', 'T', '\r', '\n'};
    // UPDATE: 将字符串改为宏定义 增加可移植度
    uint8_t RST[] = "AT+RST\r\n";
    uint8_t CWMODE[] = "AT+CWMODE=2\r\n";
    uint8_t CWSAP[] = "AT+CWSAP=\"g202001\",\"12345678\",1,3\r\n";
#if USE_PROTOCOL == USE_UDP
    uint8_t CIPSTART[] = "AT+CIPSTART=\"UDP\",\"192.168.4.255\",2345,2345,0\r\n";
    uint8_t CIPMODE[] = "AT+CIPMODE=1\r\n";
    uint8_t CIPSEND[] = "AT+CIPSEND\r\n";
#else
    uint8_t CIPMUX[] = "AT+CIPMUX=1\r\n";
    uint8_t CIPSERVER[] = "AT+CIPSERVER=1,8080\r\n";
#endif

    set_opt(fd, 115200, 8, 'n', 1);
    write_port(fd, RST, 8);
    sleep(DELAY_SEC);
    write_port(fd, CWMODE, 13);
    sleep(DELAY_SEC);
    write_port(fd, RST, 8);
    sleep(DELAY_SEC);
    write_port(fd, CWSAP, 43);
    sleep(DELAY_SEC);
    write_port(fd, RST, 8);
    sleep(DELAY_SEC);
#if USE_PROTOCOL == USE_UDP
    write_port(fd, CIPSTART, strlen(CIPSTART));
    sleep(DELAY_SEC);
    write_port(fd, CIPMODE, strlen(CIPMODE));
    sleep(DELAY_SEC);
    write_port(fd, CIPSEND, strlen(CIPSEND));
    sleep(DELAY_SEC);
#else
    //char buf[300];
    //printf("Start to config wifi......\n");
    write_port(fd, CIPMUX, strlen(CIPMUX));
    sleep(DELAY_SEC);
    write_port(fd, CIPSERVER, strlen(CIPSERVER));
    sleep(DELAY_SEC);
#endif
    tcflush(fd, TCIOFLUSH); // 刷新输入输出缓冲区
    return 0;
}

int getespinfo()
{
}

int getespsta()
{
}
