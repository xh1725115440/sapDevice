//
// 通用USB串口控制v1.0 2018/7/17
// 完成通用USB串口控制基本程序，只能打开不大于9个串口。
// 通用USB串口控制v2.0 2019/3/1
// 消除USB串口9个限制，最大能打开100个端口，完善错误返回。  
//
// TODO：给出错误编号，替代-1错误 
//
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "usbctl.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
// 串口/端口打开[阻塞方式]
// @param
// @comport 需要打开的USB端口号 
// Lora标识0，tong_lora
// WIFI标识1，tong_wifi
// 蓝牙标识2，tong_bt
// 4G标识3，  tong_4g
// RS485 4   /dev/ttymxc4    ttymxc7为GPS串口
// @return  打开失败返回< -1 >
//          打开成功返回对应的< 文件描述符fd>0 >
int open_port(int comport)
{
    int fd;
    if (0 == comport)
    {
        fd = open("/dev/tong_lora", O_RDWR | O_NOCTTY | O_NDELAY);//读写模式，防止设备成为控制终端，非阻塞模式
        if (-1 == fd)
        {
            printf("Open lora fail!\n");
        }
    }
    else if (1 == comport)
    {
        fd = open("/dev/tong_wifi", O_RDWR | O_NOCTTY | O_NDELAY);
        if (-1 == fd)
        {
            printf("Open wifi fail!\n");
        }
    }

    else if (2 == comport)
    {
        fd = open("/dev/tong_bt", O_RDWR | O_NOCTTY | O_NDELAY);
        if (-1 == fd)
        {
            printf("Open bt fail!\n");
        }
    }
    else if (3 == comport)
    {
        fd = open("/dev/tong_4g", O_RDWR | O_NOCTTY | O_NDELAY);
        if (-1 == fd)
        {
            printf("Open 4g fail!\n");
        }
    }
    else if (4 == comport)  //传感器阵列的RS485端口 中间一个
    {
        fd = open("/dev/ttymxc2", O_RDWR | O_NOCTTY | O_NDELAY);
        if (-1 == fd)
        {
            printf("Open ttymxc2 fail!\n");
        }
    }
    else if (5 == comport) // RS485左边的一个
    {
        fd = open("/dev/ttymxc4", O_RDWR | O_NOCTTY | O_NDELAY);
        if (-1 == fd)
        {
            printf("Open ttymxc4 fail!\n");
        }
    }
    else if (6 == comport) //RS485最右边的一个
    {
        fd = open("/dev/ttymxc1", O_RDWR | O_NOCTTY | O_NDELAY);
        if (-1 == fd)
        {
            printf("Open ttymxc1 fail!\n");
        }
    }
    else if (7 == comport) //RS232端口
    {
        fd = open("/dev/ttymxc3", O_RDWR | O_NOCTTY | O_NDELAY);
        if (-1 == fd)
        {
            printf("Open ttymxc3 fail!\n");
        }
    }
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        return (-1);
    }

    // 恢复串口为阻塞状态
    // 设置给arg描述符状态标志，可以更改的几个标志是O_APPEND,O_NONBLOCK,O_SYNC,O_ASYNC
    if (fcntl(fd, F_SETFL, 0) < 0)
        printf("fcntl failed!\n");
    else
        //printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
    // 测试是否为终端设备

    //if (isatty(STDIN_FILENO) == 0)
    //   printf("standard input is not a terminal device\n");
    //else
        //printf("isatty success!\n");
    //printf("Device descriptor = %d\n", fd);
        return fd;
}

// 串口配置
// @param
// @fd      文件描述符fd
// @nSpeed  波特率选项< 2400 4800 9600 115200 460800 > 若不为上述选项，则自动设为9600
// @nBits   数据位数选项 < 7 8 >
// @nEvent  奇偶检验为选项 < o e n>
// @nStop   设置停止位 < 1 2 >
// @return  错误代码
//
int set_opt(int fd, int nSpeed, int nBits, uint8_t nEvent, int nStop)
{
    struct termios newtio, oldtio;
    // 保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("SetupSerial 1");
        printf("tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(fd, &oldtio));
        return -1;
    }
    bzero(&newtio, sizeof(newtio)); // 类似memset
    // 设置字符大小
    newtio.c_cflag |= (CLOCAL | CREAD);
    newtio.c_cflag &= ~CSIZE;
    /*新加的*/
   //newtio.c_cflag |= CRTSCTS;  //使用硬件流控制
   //newtio.c_oflag &= ~(OPOST);  
   //newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
   //newtio.c_iflag &= ~(IXON|IXOFF|IXANY); //使用软件流控制
   //newtio.c_iflag |=~ICANON;
    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }
    // 设置奇偶校验位
    switch (nEvent)
    {
    case 'o':
    case 'O': // 奇数
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'e':
    case 'E': // 偶数
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'n':
    case 'N': // 无奇偶校验位
        newtio.c_cflag &= ~PARENB;
        newtio.c_iflag &= ~INPCK;

        break;
    default:
        break;
    }
    // 设置波特率
    switch (nSpeed)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    // 设置停止位
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    // 设置等待时间和最小接收字符[该参数可根据需要调整，影响程序的阻塞特性]
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    // 处理未接收字符
    tcflush(fd, TCIFLUSH);
    // 激活新配置
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("com set error");
        return -1;
    }
    //printf("set done!\n");
    return 0;
}

// 串口配置 主要用于485端口的端口配置
// @param
// @fd      文件描述符fd
// @nSpeed  波特率选项< 2400 4800 9600 115200 460800 > 若不为上述选项，则自动设为9600
// @nBits   数据位数选项 < 7 8 >
// @nEvent  奇偶检验为选项 < o e n>
// @nStop   设置停止位 < 1 2 >
// @falg    是否开启硬件流控制 1开启 0不开启
// @return  错误代码
//
int set_opt(int fd, int nSpeed, int nBits, uint8_t nEvent, int nStop, int flag)
{
    struct termios newtio, oldtio;
    // 保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("SetupSerial 1");
        printf("tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(fd, &oldtio));
        return -1;
    }
    bzero(&newtio, sizeof(newtio)); // 类似memset
    // 设置字符大小
    newtio.c_cflag |= (CLOCAL | CREAD);
    newtio.c_cflag &= ~CSIZE;
    switch (flag)
    {
    case 0:
        break;
    case 1:
        newtio.c_cflag |= CRTSCTS;  //使用硬件流控制
        break;
    }
    /*新加的*/
    //newtio.c_cflag |= CRTSCTS;  //使用硬件流控制
    //newtio.c_oflag &= ~(OPOST);  
    //newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 
    //newtio.c_iflag &= ~(IXON|IXOFF|IXANY); //使用软件流控制
    //newtio.c_iflag |=~ICANON;
    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    default:
        break;
    }
    // 设置奇偶校验位
    switch (nEvent)
    {
    case 'o':
    case 'O': // 奇数
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'e':
    case 'E': // 偶数
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'n':
    case 'N': // 无奇偶校验位
        newtio.c_cflag &= ~PARENB;
        newtio.c_iflag &= ~INPCK;

        break;
    default:
        break;
    }
    // 设置波特率
    switch (nSpeed)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    // 设置停止位
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    // 设置等待时间和最小接收字符[该参数可根据需要调整，影响程序的阻塞特性]
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    // 处理未接收字符
    tcflush(fd, TCIFLUSH);
    // 激活新配置
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("com set error");
        return -1;
    }
    //printf("set done!\n");
    return 0;
}
// 写串口
// @param
// @fd      文件描述符fd
// @buf     需写入的缓存首地址（需发送的数据/组）
// @nBits   写入的缓存大小 单位KB字节
// @return  失败返回< -1 >
//          成功返回已发送的字节长度
//
int write_port(int fd, const void* buf, size_t len)
{
    int ret;
    ret = write(fd, buf, len);
    if (ret < 0)
        return ret;
    return ret;
}

// 写串口
// @param
// @fd      文件描述符fd
// @buf     读取得数据存储的缓存首地址
// @nBits   读取的数据大小 单位KB字节 必须小于给出的缓存大小
// @return  失败返回< -1 >
//          成功返回已读取的字节长度
//
int read_port(int fd, void* buf, size_t len)
{
    int ret;
    ret = read(fd, buf, len);
    if (ret < 0)
        return ret;
    tcflush(fd, TCIFLUSH);
    return ret;
}

// 读后写串口
// @param
// @fd      文件描述符fd
// @wbuf    需写入的缓存首地址（需发送的数据/组）
// @wlen    写入的缓存大小 单位KB字节
// @rbuf    读取得数据存储的缓存首地址
// @rlen    读取的数据大小 单位KB字节 必须小于给出的缓存大小
// @return  失败返回< -1 >
//          成功返回已读取的字节长度
int write_read(int fd, void* wbuf, size_t wlen, void* rbuf, size_t rlen)
{
    int ret;
    ret = write(fd, wbuf, wlen);
    if (ret < 0)
        return ret;
    ret = read(fd, rbuf, rlen);
    if (ret < 0)
        return ret;
    return 0;
}