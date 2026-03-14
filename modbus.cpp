#include <stdio.h>                      //  C中的标准输入/输出(printf)
#include <iostream>                     //  C++中的输入输出流(cout)
#include <stdlib.h>                     //  如动态内存分配和释放、类型转换、随机数生成、排序等
#include <unistd.h>                     //  进程相关，如 fork() getpid()
#include <math.h>                       //  数学计算
#include <fcntl.h>                      //  C中的文件相关操作
#include <fstream>                      //  C++中的文件相关操作
#include <sys/types.h>                  //  提供了一些特定于系统的类型定义，如 pid_t
#include <sys/socket.h>                 //  网络编程相关，如 socket
#include <sys/time.h>                   //  时间相关
#include <sys/stat.h>                   //  文件状态、权限相关
#include <sys/select.h>                 //  I/O多路复用，如 fd_set select()
#include <arpa/inet.h>                  //  互联网地址相关，如 inet_addr()
#include <thread>                       //  线程相关
#include <pthread.h>                    //  多线程相关
#include <mutex>                        //  线程中的同步和互斥操作
#include <string>                       //  字符串相关
#include <vector>                       //  动态数组
#include <queue>                        //  队列
#include <iomanip>                      //  输入输出流格式化
#include <memory.h>                     //  内存操作相关
#include <errno.h>                      //  错误代码和信息
#include "/home/wp/guoxiang/apDevice/libmodbus/include/modbus/modbus.h"
#include "driver/lora.h"  
#include "driver/bluetooth.h"
#include "driver/usbctl.h"
#include "driver/get.h"
#include "driver/protocol_process.h"
#include "driver/softwarewdt.h" 
#include "driver/communicaManage.h" 
#include "driver/registManage.h" 
#include "driver/usart.h"
#define DEBUG                                           //是否打开调试模式
#define WIFI_SERVER_IP          "192.168.3.1"           //wifi服务器ip
#define WIFI_SERVER_PORT        1234                    //wifi服务器端口
#define LAN_SERVER_IP           "192.168.2.1"           //有线服务器ip
#define LAN_SERVER_PORT         2234                    //有线服务器端口
#define MODBUS_TTY              "/dev/ttymxc2"          //rs485写入串口
#define BAUD_RATE               9600                    //modbus串口波特率
#define SLAVE_ADDR              1                       //从机地址
#define RX_SIZE                 58                      //最大协议包字节数
#define MAX_DATA_SIZE           43                      //最大数据包字节数
#define ACCESS_NET_COUNT        5                       //请求入网次数
#define MAX_FD(x,y) (x > y ? x : y)                     //两者比较，找最大值
#define timeout                 3                       //设置connect连接超时时间
#define ZD_LAN_SERVER_IP        "192.168.31.101"         //本地充当服务器RJ45端口ip   "192.168.31.101"
#define ZD_LAN_SERVER_PORT      3234                    //端口号
//定义协议标识
#define REQ_REGISTER            "01"                    //注册请求
#define REQ_REG_CONFIRM         "02"                    //注册确认
#define REQ_DATA                 "03"                    //数据请求
#define REQ_TXD_CONFIRM         "04"                    //数据上传确认
#define REQ_PARAM_CONFIRM       "05"                    //传感器参数变更执行确认
#define REQ_PARAM_SEND          "06"                    //传感器参数变更命令下发
#define REQ_ID_CONFIRM          "07"                    //接入设备ID变更执行确认
#define REQ_ID_SEND             "08"                    //接入设备ID变更命令下发
#define REQ_MONI_RET_CONFIRM    "09"                    //监测终端重置执行确认
#define REQ_MONI_RET            "10"                    //监测终端重置命令下发
#define REQ_ACC_RET_CONFIRM     "11"                    //接入设备重启执行确认
#define REQ_ACC_RET             "12"                    //接入设备重启命令下发
#define REQ_KEY_CONFIRM         "13"                    //更新通信秘钥执行确认
#define REQ_KEY_SEND            "14"                    //更新通信秘钥命令下发
#define REQ_ASK_KEY             "15"                    //请求通信秘钥
#define REQ_SEND_KEY            "16"                    //下发通信秘钥
#define REQ_SEND_INFO           "17"                    //网关参数下发
#define REQ_ACC_CLOSE           "18"                    //接入设备退网
#define REQ_GATEWAY_CLOSE       "19"                    //网关设备退网
#define TIME_SEND               "20"                    //时间戳下发
#define REQ_ACC_HEART           "EF"                    //接入设备心跳包
#define REQ_GATE_HEART          "FF"                    //网关设备心跳包
//定义通信设备管理ID号
#define LORA                    1                       //lora的ID号
#define WIFI                    2                       //wifi的ID号
#define BT                      3                       //蓝牙的ID号
#define LAN                     4                       //有线连接的ID号
//定义SAP的端口号信息
#define RS485_A                 "100"     
#define RS485_B                 "010"  
#define RS485_C                 "001"   
#define RS232                   "101"
#define RJ45                    "111"
#define watchPathName "/home/root/myWatch.txt"
/**********************************************************************
(1)最新无错误版本版本SAP
(2)加入跟ISR有线连接功能，无人艇采用有线连接传输数据
    在trans（）中增加lan通信设备描述符，增加device_regist_lan();
(3)加入气象仪传感器数据处理函数
(4)9.29 加入软件看门狗检测线程的存活状态，提高程序的稳定性
(5)10.6 加入485串口监听功能，接收到数据不做处理直接上传到网关（还未完善，后续可以加入）
(6)写系统日志功能
(7)11.18 加入通信设备管理类功能，提高设备的注册能力
(8)12.5  完善三个485串口的监听
(9)12.20 加入未发送消息存入本地txt文档，在以后的时间里发送
(10) 3月31号 数据包的封装 加入设备的端口号连接信息
**********************************************************************/
using namespace std;
const int softdogTimeout=30;
int fd_lora = -1, fd_wifi = -1, fd_bt = -1, fd_lan = -1,fdL_lan=-1;//通信设备描述符
vector<int>device_id;                                   //设备描述符容器
int fd_num;                                             //文件描述符数量
string id = "FF", net_id = "0000", mac="", s_req_fre, bt_mac = "",Isr_mac="";      //设备id，设备网络id，设备mac，问询频率,Isr_mac地址
int monitor_time = 4;
string current_time = "";
char communicate_status[] = "0000";                     //通信方式描述数组
int frequency = 0;
unsigned int hj_crc; 
/*************************定义的一些全局共享变量----->进行线程之间的控制*************************/
int iS_getsensor=0; int iS_getshipsensor=0;
/************************************为线程同步定义的锁和条件变量*****************************/
static pthread_mutex_t mtx_sensor=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_sensor=PTHREAD_COND_INITIALIZER;
/*******************************************************************************************/                                   

/******************************初始化一个通信设备管理类***************************************/
communicaManage* CM =new communicaManage;
/*******************************************************************************************/  
pthread_t tid_getSensor, tid_getLan, tid_transMessage, tid_gps, tid_ship_data, tid_softwd, tid_monitor485, tid_deviceRegist;
int wdtNewSensorId = -1, wdtShipId = -1;                //存储大气和无人船线程在看门狗注册的id号
string hj_data_packet = "";                            //hj212数据包
char send_data_packet[1500];                            //大气私有协议数据包
char air_data_packet[1500];
char hj_njdata_packet[1500];                           //水土气私有协议数据包
char hj_njdata_rece[1500];                             //南京传过来的数据包
//char ship_getpacket[1500];                           //无人船传感器数据包
queue<string>send_message;                             //消息队列：发送消息
queue<string>recv_message;                             //消息队列：接收消息
//queue<string>recvnj_message;                           //消息队列：存放接收南水所的消息（作为缓存使用）
queue<string>shipsensor_message;                       //消息队列：存放无人船的数据包
//queue<string>rs485_message;                            //消息队列：存放其余两个485串口收到的消息
queue<string>transMessage;                             //缓冲区队列：用于存放采集到、收到的数据信息，然后等待发送
queue<string>readMessage;                               //缓冲区队列：用于存放文件中上一轮中未发送的数据
string len_j212="";                                    //记录总的HJ212数据包长度
string cpu_occupy="";

string gps_lat= "NFFFF";
string gps_lon="EFFFFF";                               //东经 //存储GPS经纬度
int   len_private1,len_private2;                       //总的大气私有封包长度、水土气私有包长度
int flag_wifi=0;                                       //WIFI连接到ISR成功的标志
int flag_nj=0;                                         //接收到南京数据的标志位
mutex mutex_gps;                                       //互斥锁，锁住gps
mutex mutex_send_message;                              //互斥锁，锁住send_message
mutex mutex_recvnj_message;                            //互斥锁，锁住消息队列

struct sockaddr_in wifi_ser_addr;                      //wifi的TCP
struct sockaddr_in lan_ser_addr;                       //lan的TCP绑定地址（ip和端口号）
struct sockaddr_in Llan_ser_addr;                      //接收南京的本地服务器IP
struct sockaddr_in Llan_cli_addr;                                             /**
                                                        *   中断函数，用户停止程序使用
                                                        *   @param signum 捕获的信号量
                                                        *   @return
                                                        *
                                                        */


/*
*确保循环数据区的长度为4位
*/
string ensureLen_4(int param)
{
    string s=to_string(param);
    string outstring="";
    if (param < 10)
    {

        outstring = outstring + "000" + s;
    }
    else if (param < 100)
    {
         outstring = outstring + "00" + s;
    }
    else if (param < 1000)
    {
        outstring = outstring + "0" + s;
    }
    else if (param < 10000)
    {
        outstring = outstring + s;
    }
    else
    {
        cout << "data have outflow" << endl;
    }
    return outstring;
}
/*
*确保循环冗余检验码的长度为4位
*并进行封装为完整的HJ212数据包
*参数param:数据区循环冗余检验码（十六进制）
*参数s：
*参数hj_origin:
*/
char* ensure_crc4_packet(unsigned int param,string s,uint8_t hj_origin[])
{   
    char hj212pacet[1024]={0};
   if(param > 4095)  //FFF
        {
            sprintf((char*)hj212pacet,"##%s%s%x", s.c_str(),hj_origin, param);
        }
        else if(param > 255)  //FF
        {
            sprintf((char*)hj212pacet, "##%s%s0%x", s.c_str(),hj_origin, param);
        }
        else if(param > 16) //F
        {
            sprintf((char*)hj212pacet, "##%s%s00%x", s.c_str(),hj_origin, param);
        }
        else
        {
            sprintf((char*)hj212pacet, "##%s%s000%x", s.c_str(),hj_origin, param);
        }
        return hj212pacet;
}


/**
*   设置connect连接超时函数
*   @param sockfd 创建的套接字
*   @param struct sockaddr *saptr 远端地址   //sockaddr_in
*   @param socklen_t salen 远端地址长度
*   @param nsec  超时时间
*   @return  -1 失败  0 成功
*
*/
int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec)
{
    int     flags, n, error;
    socklen_t   len;        //类似于int
    fd_set  rset, wset;     //long类型的数组 将多个套接字集合起来，并通过系统调用select来等待这些套接字上的事件
    struct timeval  tval;

    // 设置 socket 为非阻塞
    if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1) {   //获取文件状态
        perror("fcntl F_GETFL");
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {   //设置文件打开方式
        perror("fcntl F_SETFL");
    }

    error = 0;
    // 发起非阻塞 connect
    if ((n = connect(sockfd, saptr, salen)) < 0) {   //成功返回0，失败返回-1
        // EINPROGRESS 表示连接建立已启动但是尚未完成
        if (errno != EINPROGRESS) {
            return -1;
        }
    }
    else if (n == 0) {
        // 连接已经建立，当服务器处于客户端所在的主机时可能发生这种情况
        goto done;
    }

    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;

    // 等待套接字变为可读或可写，在 select 上等待连接完成
    if ((n = select(sockfd + 1, &rset, &wset, NULL, nsec ? &tval : NULL)) == 0) {  //返回值是满足事件的总个数         
        // select 返回0，说明超时发生，需要关闭套接字，以防止已经启动的三次握手继续下去
        close(sockfd);
        errno = ETIMEDOUT;  //超时
        return -1;
    }
    else if (n == -1) {
        close(sockfd);
        perror("select");
        return -1;
    }

    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        len = sizeof(error);
        // 获取待处理错误，如果建立成功，error 为0；
        // 如果连接建立发生错误，该值就是对应错误的 errno 值
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
            // Berkeley 实现将在 error 中返回待处理错误，getsocket 本身返回 0
            // Solaris 实现将 getsocket 返回 -1，并把 errno 变量设置为待处理错误
            return -1;
        }
    }
    else {
        fprintf(stderr, "select error: socket not set");
    }


done:

    // 关闭非阻塞状态
    if (fcntl(sockfd, F_SETFL, flags) == -1) {
        perror("fcntl");
    }

    if (error) {
        close(sockfd);
        errno = error;
        return -1;
    }

    return 0;
}

int lora_reinit(int fd_lora) //重新初始化lora通信设备
{

    close(fd_lora);//先关闭再打开
    fd_lora = lora_open();
    if (fd_lora == -1) 
    {
        cout << "lora重新初始化失败" << endl;
        return -1;
    }
    else
    {   
        communicate_status[0] = '1';
        cout << "lora重新初始化成功" << endl;
        return fd_lora;
    }
    
}
int wifi_reinit(int fd_wifi)
{   
    close(fd_wifi);
    //wifi设备初始化
    system("ifconfig wlan0 down");               //先关闭无线网卡wlan0
    sleep(2);
    system("killall wpa_supplicant");            //关闭所有wpa_supplicant进程
    sleep(1);
    system("killall udhcpc");                    //关闭所有udhcpc进程
    sleep(1);
    system("wpa_supplicant -i wlan0 -B -c /home/root/wifi.conf &"); //连接wifi进程，并放到后台运行。 wifi.conf里面包括WiFi密码及名字   -B 表示该进程为守护进程
    sleep(10);
    system("udhcpc -i wlan0 -t 8 -n");                 //获取ip地址进程，最多发送8个发现包，没有获得ip则退出
    int fd_wifi_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_wifi_sock < 0)
    {
        printf("wifi: ======fd_wifi_sock create failure");
        return -1;
    }
    else
    {
        printf("wifi: >>>>>>>>>>>>>>>>>>>>>>>>>>>");
        printf("wifi: fd_wifi_sock create success\n");
        memset(&wifi_ser_addr, 0x0, sizeof(struct sockaddr_in));
        wifi_ser_addr.sin_family = AF_INET;                             //ipv4协议族
        wifi_ser_addr.sin_port = htons(WIFI_SERVER_PORT);               //指定目标机端口:1234
        wifi_ser_addr.sin_addr.s_addr = inet_addr(WIFI_SERVER_IP);      //指定目标机ip:192.168.3.1
        int wifi_fd_connect;                                            //wifi_fd_connect是判断是否链接服务器成功的返回值（非文件描述符）
        wifi_fd_connect = connect_nonb(fd_wifi_sock, (struct sockaddr *)&wifi_ser_addr, sizeof(struct sockaddr_in), timeout);
        if (wifi_fd_connect < 0)
        {
            perror("wifi: connect");
            system("killall wpa_supplicant");
            system("killall udhcpc");
            return -1;
        }
        else
        {
            printf("wifi: connect server success\n");
            printf("Init success: wifi. Device descriptor = %d\n", fd_wifi_sock);
            fd_wifi = fd_wifi_sock;
            communicate_status[1] = '1';
            system("echo 1 > /sys/class/leds/yellow/brightness");      //wifi正常打开，黄灯亮
            return fd_wifi;
        }
    }
}

int bt_reinit(int fd_bt)
{
    close(fd_bt);
    fd_bt = bluetooth_open(bt_mac);
    if (-1 == fd_bt)
    {
        bt_mac = "";
        printf("-> No blue_tooth device\n");
        return -1;
    }
    else
    {
        printf("Init success：bluetooth. Device descriptor = %d\n", fd_bt);
        communicate_status[3] = '1';
        system("echo 1 > /sys/class/leds/green/brightness");
        return fd_bt;
    }
}

int lan_reinit(int fd_lan)
{
    close(fd_lan);
    system("ifconfig eth1 192.168.2.235 netmask 255.255.255.0");
    sleep(2);
    //开始绑定：监听192.168.2.2，端口:2234
    fd_lan = socket(AF_INET, SOCK_STREAM, 0);    //创建套接字
    if (fd_lan < 0)
    {
        printf("lan: socket create error: %s\n", strerror(errno));
        return -1;
    }
    else
    {
        memset(&lan_ser_addr, 0, sizeof(lan_ser_addr));
        lan_ser_addr.sin_family = AF_INET;
        //svraddr.sin_addr.s_addr = INADDR_ANY;
        lan_ser_addr.sin_addr.s_addr = inet_addr(LAN_SERVER_IP);
        lan_ser_addr.sin_port = htons(LAN_SERVER_PORT);
        printf("Init success: Lan. Device descriptor = %d\n", fd_lan);
        //int lan_fd_connect = connect_nonb(fd_lan,(struct sockaddr*)&lan_ser_addr,sizeof(lan_ser_addr), timeout);    //连接服务器
        int lan_fd_connect = connect(fd_lan, (struct sockaddr*)&lan_ser_addr, sizeof(lan_ser_addr));
        if (lan_fd_connect < 0)
        {
            perror("lan: connect");
            return -1;
        }
        else
        {
            printf("lanship: connect server success\n");
            printf("Init success: Lan. Device descriptor = %d\n", fd_lan);
            communicate_status[2] = '1';
            return fdL_lan;
        }
    }
}


/**
*   设备初始化
*   lora blue wifi lan
*   @return  -1 失败  0 成功
*
*/
int dev_init()
{
    //初始化lora
    fd_lora = lora_open();
    if (-1 == fd_lora)               //将判断参数写在左边，可以避免少写等号从而变成赋值
    {
#ifdef DEBUG 
        printf("--> No lora device\n");
#endif
    }
    else 
    {
        device_id.push_back(fd_lora);
        CM->addCommunicateNode(LORA,fd_lora);  //将初始化成功的LORA设备添加至管理器
        CM->callbackRgist(LORA,0,lora_reinit); //注册LORA的重新初始化函数
        printf("Init success: lora. Device descriptor = %d\n", fd_lora);
        communicate_status[0] = '1';
        system("echo 1 > /sys/class/leds/red/brightness");
    }
    sleep(2);
    //-----------------------------------------------------------
    //wifi设备初始化
    system("ifconfig wlan0 down");               //先关闭无线网卡wlan0
    sleep(2);
    system("killall wpa_supplicant");            //关闭所有wpa_supplicant进程
    sleep(1);
    system("killall udhcpc");                    //关闭所有udhcpc进程
    sleep(1);
    system("wpa_supplicant -i wlan0 -B -c /home/root/wifi.conf &");                 //连接wifi进程，并放到后台运行。 wifi.conf里面包括WiFi密码及名字   -B 表示该进程为守护进程
    sleep(10);
    system("udhcpc -i wlan0 -t 8 -n");                 //获取ip地址进程，最多发送8个发现包，没有获得ip则退出
    sleep(5);
    int fd_wifi_sock = socket(AF_INET, SOCK_STREAM, 0);                //创建TCP套接字,AF_INET：ipv4协议族；SOCK_STREAM：TCP字节流 SOCK_DGRAM UDP
    if (fd_wifi_sock < 0)
    {
#ifdef DEBUG 
        printf("wifi: ======fd_wifi_sock create failure");
    
#endif 
    }
    else
    {
#ifdef DEBUG
        printf("wifi: >>>>>>>>>>>>>>>>>>>>>>>>>>>");
        printf("wifi: fd_wifi_sock create success\n");
        //printf("wifi: fd_wifi_sock: %d\n", fd_wifi);
#endif

        memset(&wifi_ser_addr, 0x0, sizeof(struct sockaddr_in));
        wifi_ser_addr.sin_family = AF_INET;                             //ipv4协议族
        wifi_ser_addr.sin_port = htons(WIFI_SERVER_PORT);               //指定目标机端口:1234
        wifi_ser_addr.sin_addr.s_addr = inet_addr(WIFI_SERVER_IP);      //指定目标机ip:192.168.3.1
        int wifi_fd_connect;

        wifi_fd_connect = connect_nonb(fd_wifi_sock, (struct sockaddr *)&wifi_ser_addr, sizeof(struct sockaddr_in), timeout);
        if(wifi_fd_connect < 0)
        {
#ifdef DEBUG 
            perror("wifi: connect");
            system("killall wpa_supplicant");
            system("killall udhcpc");
#endif 
        }
        else
        {
#ifdef DEBUG
            printf("wifi: connect server success\n");
            printf("Init success: wifi. Device descriptor = %d\n", fd_wifi_sock);
            flag_wifi=1;                                              //WIFI初始化成功且成功连接到ISR
#endif
            fd_wifi = fd_wifi_sock;
            CM->addCommunicateNode(WIFI,fd_wifi);
            CM->callbackRgist(WIFI,0,wifi_reinit);
            device_id.push_back(fd_wifi);
            communicate_status[1] = '1';
            system("echo 1 > /sys/class/leds/yellow/brightness");      //wifi正常打开，黄灯亮
        }
    }


    //      int wifi_fd_connect;
    //      //因为connect本身不具备超时设置，使用select来控制时间
    //      wifi_fd_connect = connect_nonb(fd_wifi, (struct sockaddr *)&wifi_ser_addr, sizeof(wifi_ser_addr), timeout);
    //      wifi_fd_connect = connect(fd_wifi,(sockaddr*)&wifi_ser_addr,sizeof(wifi_ser_addr));    //连接服务器
    //      if(wifi_fd_connect < 0)
    //      {
    // #ifdef DEBUG 
    //          perror("wifi: connect");
    //             system("killall wpa_supplicant");
    //             system("killall udhcpc");
    // #endif 
    //         }
    //      else
    //      {
    // #ifdef DEBUG
    //          printf("wifi: connect server success\n");
    //             printf("Init success: wifi. Device descriptor = %d\n", fd_wifi_sock);
    // #endif
    //          device_id.push_back(fd_wifi_sock);
    //             fd_wifi = fd_wifi_sock;
    //             system("echo 1 > /sys/class/leds/yellow/brightness");      //wifi正常打开，黄灯亮
    //      }

    //--------------------------------------  
    //blue_tooth初始化
    /**蓝牙设备初始化**/
    fd_bt = bluetooth_open(bt_mac);
    if (-1 == fd_bt)
    {
        bt_mac = "";
        printf("-> No blue_tooth device\n");
    }
    else
    {   
        CM->addCommunicateNode(BT,fd_bt);
        CM->callbackRgist(BT,0,bt_reinit);
        device_id.push_back(fd_bt);
#ifdef DEBUG 
        printf("Init success：bluetooth. Device descriptor = %d\n", fd_bt);
#endif
        communicate_status[3] = '1';
        system("echo 1 > /sys/class/leds/green/brightness");
    }
    sleep(2);

    //-----------------------------------------------

    //初始化网口1，作为服务器端口使用，接收来自其他节点的数据
    //设备的ip
    system("ifconfig eth0 192.168.31.101 netmask 255.255.255.0");
    sleep(2);
    fdL_lan=socket(AF_INET,SOCK_STREAM,0);//创建服务器套接字
     if (fdL_lan < 0)
    {
#ifdef DEBUG
        printf("lan: socket create error: %s\n", strerror(errno));
#endif 
    }
    else
    {
        memset(&Llan_ser_addr, 0, sizeof(Llan_ser_addr));
        Llan_ser_addr.sin_family = AF_INET;
        //svraddr.sin_addr.s_addr = INADDR_ANY;
        Llan_ser_addr.sin_addr.s_addr = inet_addr(ZD_LAN_SERVER_IP); //inet_addr
        Llan_ser_addr.sin_port = htons(ZD_LAN_SERVER_PORT);
        bind(fdL_lan,(struct sockaddr *)&Llan_ser_addr,sizeof(Llan_ser_addr));
        listen(fdL_lan,5);
#ifdef DEBUG
        printf("Init success: LanLSS. Device descriptor = %d\n", fdL_lan);
#endif
    }
    //-----------------------------------------------

    //初始化网口2，作为客户端使用，与ISR进行有线方式的注册连接
    system("ifconfig eth1 192.168.2.235 netmask 255.255.255.0");
    sleep(2);
    //开始绑定：监听192.168.2.2，端口:2234
    fd_lan = socket(AF_INET, SOCK_STREAM, 0);    //创建套接字
    if (fd_lan < 0)
    {
#ifdef DEBUG
        printf("lan: socket create error: %s\n", strerror(errno));
#endif 
    }
    else
    {
        memset(&lan_ser_addr, 0, sizeof(lan_ser_addr));
        lan_ser_addr.sin_family = AF_INET;
        //svraddr.sin_addr.s_addr = INADDR_ANY;
        lan_ser_addr.sin_addr.s_addr = inet_addr(LAN_SERVER_IP);
        lan_ser_addr.sin_port = htons(LAN_SERVER_PORT);
#ifdef DEBUG
        //printf("lan: connect server success\n");
        printf("Init success: Lan. Device descriptor = %d\n", fd_lan);
#endif
        //int lan_fd_connect = connect_nonb(fd_lan,(struct sockaddr*)&lan_ser_addr,sizeof(lan_ser_addr), timeout);    //连接服务器
        int lan_fd_connect=connect(fd_lan,(struct sockaddr*)&lan_ser_addr,sizeof(lan_ser_addr));
        if(lan_fd_connect < 0)
        {
#ifdef DEBUG 
            perror("lan: connect");
#endif 
        }
        else
        {
#ifdef DEBUG
            printf("lanISR: connect server success\n");
            printf("Init success: Lan. Device descriptor = %d\n", fd_lan);
#endif
            CM->addCommunicateNode(LAN,fd_lan);
            CM->callbackRgist(LAN,0,lan_reinit);
            device_id.push_back(fd_lan);
            communicate_status[2] = '1';
        }       
    }



    //检测是否设备成功被初始化
    if (device_id.empty())
    {
        return -1;
    }
    return device_id.size();
}

//指示灯操作函数
void close_led()
{
    system("echo 0 > /sys/class/leds/red/brightness");
    system("echo 0 > /sys/class/leds/yellow/brightness");
    system("echo 0 > /sys/class/leds/green/brightness");
}


/**
*   获取设备唯一mac地址
*   从文件中读取
*   @return
*
*/
void get_mac()
{
    FILE *fp;  //指向文件的指针变量
    fp = freopen("./mac.txt", "r", stdin); //打开文件，并将标准输入(stdin)重定向到该文件
    if (fp == NULL)
    {
        printf("get mac failed\n");
    }
    else
    {
        getline(cin, mac); //从标准输入(cin)中读取一行文本，并将其存储在名为"mac"的字符串变量中
    }
    fclose(stdin);
    fclose(fp);
    return;
}



/**
*   获取气体模组采集设备唯一标识MN
*   从文件中读取
*   @return
*
*/
string get_MN()
{
    string temp = "";
    char c[100];
    FILE *fptr;
    if ((fptr = fopen("./hj_mn.txt", "r")) == NULL)
    {
        printf("Error！ opening file\n");
    }
    fscanf(fptr, "%[^\n]", c);
    temp = temp + c;
    fclose(fptr);
    return temp;
}



/**
*   设备注册确认
*   @return  -1 失败  0 成功
*
*/

void device_OldRegist()
{   
    cout << "device_regist" << endl;
    if(CM->getSize()==0)                        //如果没有初始化成功的通信设备
    {

        system("reboot");
            /*********************************************执行相关的操作***************************************************/
    }                                                                                                                                        
    vector<vector<int>>vec =CM->getALLIfd();    //获取初始化成功的通信设备，按照优先级排序
    struct timeval tv1;
    struct timeval tv2;
    int ret;
    fd_set fdset;                               //文件描述符集合
    FD_ZERO(&fdset);
    FD_SET(fd_lora, &fdset);
    uint8_t RX_buf[RX_SIZE];                    //接收字符串
    string regist_message = "";                 // string accessdev_gps = gps_lat + "," + gps_lon;
    regist_message = regist_message + "$" + "01" + "1" + id + net_id + "00" + "0026" + mac + communicate_status +"010"+ gps_lon+gps_lat+cpu_occupy+"@";  //注册请求消息

    int select_num = 1;
    //开始监听信道
select_again:
    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(fd_lora, &fdset);
        tv1.tv_sec = 15;
        tv1.tv_usec = 0;
        ret = select(fd_lora + 1, &fdset, NULL, NULL, &tv1);  //监听15s
        if (ret > 0)
        {
            //监听到一个包
            //测试fd_lora是否可读
            if (FD_ISSET(fd_lora, &fdset))
            {
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read(fd_lora, RX_buf, RX_SIZE);
                if (ret < 58 && RX_buf[0] != '$' && RX_buf[ret - 1] == '@') //该包为最后一个数据包
                {
                    
                    goto sendreg_entrance;
                }
                else
                {
                    if(RX_buf[0] == '$' && RX_buf[ret - 1] == '@' && select_num == 5)
                    {
                        goto sendreg_entrance;
                    }
                    else
                    {
                        select_num++;
                    }
                }
            }

        }
        else if (0 == ret)   //此时信道空闲，可以直接发送注册信息
        {
            goto sendreg_entrance;
        }
    }


sendreg_entrance:
    //向LORA模块写入注册请求数据
    sleep(monitor_time / 4);   //暂停四分之一个监听周期
    ret = write(fd_lora, regist_message.c_str(), strlen(regist_message.c_str()));      //写入注册请求命令
    if(ret <= 0)
    {
#ifdef DEBUG
        cout << "device_regist: write error:" << strerror(errno) << endl;
#endif
        select_num = 1;
        goto select_again;
    }
    else   //成功写入注册请求数据，开始监听网关是否响应
    {
#ifdef DEBUG
        cout << "ret:" << ret << endl;
        cout << "Regist: " << regist_message << endl;
#endif
        int i = 5;
        while (i--)
        {

            FD_ZERO(&fdset);
            FD_SET(fd_lora, &fdset);
            tv2.tv_sec = monitor_time;
            tv2.tv_usec = 0;
            ret = select(fd_lora + 1, &fdset, NULL, NULL, &tv2);
            cout << "ret >> " << ret << endl;
            if (ret > 0)
            {
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read(fd_lora, RX_buf, RX_SIZE);
#ifdef DEBUG
                cout << "Regist recv：" << RX_buf << endl;
#endif 
                //对于接收到的数据进行分析处理
                string regist_recv_message(RX_buf, RX_buf + ret);
                string recv_protocal = regist_recv_message.substr(1, 2);
                if (recv_protocal == REQ_SEND_INFO)
                {
                    //接收到的是参数下发协议，则判断下发的数据包里面的mac是否匹配
                    string recv_mac = regist_recv_message.substr(16, 16);   //获取下发的mac地址，设备唯一码
                    if (recv_mac == mac)                                    //确认是该接入设备发起的注册请求
                    {
                        Isr_mac=regist_recv_message.substr(32,16);          //获取网关设备ISR的MAC地址
                        id = regist_recv_message.substr(48, 2);             //获取下发的id
                        net_id = regist_recv_message.substr(50, 4);          //获取所在网络的id
                        /*current_time = regist_recv_message.substr(54, 17);   //获取网关下发参数时间戳 2016 08 01 08 58 57 223
                        string year_time_info = current_time.substr(0, 4);
                        string month_time_info = current_time.substr(4, 2);
                        string day_time_info = current_time.substr(6, 2);
                        string hour_time_info = current_time.substr(8, 2);
                        string minute_time_info = current_time.substr(10, 2);
                        string sec_time_info = current_time.substr(12, 2);
                        string msec_time_info = current_time.substr(14, 3);
                        char time[50] = "";
                        sprintf(time, "date %s-%s-%s", year_time_info.c_str(), month_time_info.c_str(), day_time_info.c_str());
                        cout << "current year :" << time << endl;
                        system(time);
                        memset(time, 0, sizeof(time));
                        sprintf(time, "date %s:%s:%s", hour_time_info.c_str(), minute_time_info.c_str(), sec_time_info.c_str());
                        cout << "current hour :" << time << endl;
                        system(time);*/
                        break;
                    }
                    else
                    {
                        //接收到的是参数下发的消息，但是mac地址不对应，则重新接收
                        continue;
                    }
                }
                else
                {
                    //接收到的消息不是参数下发的，则重新接收
                    continue;
                }
            }
        }
        //证明已经更新了设备的id
        if (id != "FF")
        {
            //设备注册成功，回传注册确认，并退出
            string confirm_message = "";
            confirm_message = confirm_message + "$" + "02" + "1" + id + net_id + "00" + "0012" + mac + id + "@";
            ret = write(fd_lora, confirm_message.c_str(), strlen(confirm_message.c_str()));
            if (ret <= 0)
            {
                printf("write confirm_message failed!");
            }

#ifdef DEBUG
            cout << "confirm_message:" << confirm_message << endl;
#endif
        }
        else
        {
            select_num = 1; 
            goto select_again;
        }
    }

}

//无人船有线的方式注册
void device_regist_lan()
{
    #ifdef DEBUG
    cout << "device_regist" << endl;
#endif
    struct timeval tv1;
    struct timeval tv2;
    int ret;
    fd_set fdset;               //文件描述符集合
    FD_ZERO(&fdset);
    FD_SET(fd_lan, &fdset);
    uint8_t RX_buf[RX_SIZE];    //接收字符串
    string regist_message = "";  // string accessdev_gps = gps_lat + "," + gps_lon;
    regist_message = regist_message + "$" + "01" + "1" + id + net_id + "00" + "0026" + mac + communicate_status +"010"+ gps_lon+gps_lat+cpu_occupy+"@";  //注册请求消息

    int select_num = 1;
    //开始监听信道
select_again:
    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(fd_lan, &fdset);
        tv1.tv_sec = 15;
        tv1.tv_usec = 0;
        ret = select(fd_lan + 1, &fdset, NULL, NULL, &tv1);  //监听15s
        if (ret > 0)
        {
            //监听到一个包
            //测试fd_lora是否可读
            if (FD_ISSET(fd_lan, &fdset))
            {
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read(fd_lan, RX_buf, RX_SIZE);
                if (ret < 58 && RX_buf[0] != '$' && RX_buf[ret - 1] == '@') //该包为最后一个数据包
                {
                    
                    goto sendreg_entrance;
                }
                else
                {
                    if(RX_buf[0] == '$' && RX_buf[ret - 1] == '@' && select_num == 5)
                    {
                        goto sendreg_entrance;
                    }
                    else
                    {
                        select_num++;
                    }
                }
            }

        }
        else if (0 == ret)   //此时信道空闲，可以直接发送注册信息
        {
            goto sendreg_entrance;
        }
    }


sendreg_entrance:
    //向LORA模块写入注册请求数据
    //sleep(monitor_time / 4);   //暂停四分之一个监听周期
    ret = write(fd_lan, regist_message.c_str(), strlen(regist_message.c_str()));      //写入注册请求命令
    if(ret <= 0)
    {
#ifdef DEBUG
        cout << "device_regist: write error:" << strerror(errno) << endl;
#endif
        select_num = 1;
        goto select_again;
    }
    else   //成功写入注册请求数据，开始监听网关是否响应
    {
#ifdef DEBUG
        cout << "ret:" << ret << endl;
        cout << "Regist: " << regist_message << endl;
#endif
        int i = 5;
        while (i--)
        {

            FD_ZERO(&fdset);
            FD_SET(fd_lan, &fdset);
            tv2.tv_sec = monitor_time;
            tv2.tv_usec = 0;
            ret = select(fd_lan + 1, &fdset, NULL, NULL, &tv2);
            cout << "ret >> " << ret << endl;
            if (ret > 0)
            {
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read(fd_lan, RX_buf, RX_SIZE);
#ifdef DEBUG
                cout << "Regist recv：" << RX_buf << endl;
#endif 
                //对于接收到的数据进行分析处理
                string regist_recv_message(RX_buf, RX_buf + ret);
                string recv_protocal = regist_recv_message.substr(1, 2);
                if (recv_protocal == REQ_SEND_INFO)
                {
                    //接收到的是参数下发协议，则判断下发的数据包里面的mac是否匹配
                    string recv_mac = regist_recv_message.substr(16, 16);   //获取下发的mac地址，设备唯一码
                    if (recv_mac == mac)                                    //确认是该接入设备发起的注册请求
                    {
                        Isr_mac=regist_recv_message.substr(32,16);          //获取网关设备ISR的MAC地址
                        id = regist_recv_message.substr(48, 2);             //获取下发的id
                        net_id = regist_recv_message.substr(50, 4);          //获取所在网络的id
                        /*current_time = regist_recv_message.substr(54, 17);   //获取网关下发参数时间戳 2016 08 01 08 58 57 223
                        string year_time_info = current_time.substr(0, 4);
                        string month_time_info = current_time.substr(4, 2);
                        string day_time_info = current_time.substr(6, 2);
                        string hour_time_info = current_time.substr(8, 2);
                        string minute_time_info = current_time.substr(10, 2);
                        string sec_time_info = current_time.substr(12, 2);
                        string msec_time_info = current_time.substr(14, 3);
                        char time[50] = "";
                        sprintf(time, "date %s-%s-%s", year_time_info.c_str(), month_time_info.c_str(), day_time_info.c_str());
                        cout << "current year :" << time << endl;
                        system(time);
                        memset(time, 0, sizeof(time));
                        sprintf(time, "date %s:%s:%s", hour_time_info.c_str(), minute_time_info.c_str(), sec_time_info.c_str());
                        cout << "current hour :" << time << endl;
                        system(time);*/
                        break;
                    }
                    else
                    {
                        //接收到的是参数下发的消息，但是mac地址不对应，则重新接收
                        continue;
                    }
                }
                else
                {
                    //接收到的消息不是参数下发的，则重新接收
                    continue;
                }
            }
        }
        //证明已经更新了设备的id
        if (id != "FF")
        {   
            int listen_num=5;
            //设备注册成功，回传注册确认，并退出
            string confirm_message = "";
            confirm_message = confirm_message + "$" + "02" + "1" + id + net_id + "00" + "0012" + mac + id + "@";
            ret = write(fd_lan, confirm_message.c_str(), strlen(confirm_message.c_str()));
            if (ret <= 0)
            {
                printf("write confirm_message failed!");
            }

#ifdef DEBUG
            cout << "confirm_message:" << confirm_message << endl;
            cout << "等待下发时间戳"<<endl;
            while(listen_num)
            {
                FD_ZERO(&fdset);
                FD_SET(fd_lan, &fdset);
                ret = select(fd_lan + 1, &fdset, NULL, NULL,NULL); //继续开始监听时间戳消息
                listen_num--;
                cout<<listen_num<<endl;
                if(ret>0)
                {
                    memset(RX_buf, 0, sizeof(RX_buf));
                    ret = read(fd_lan, RX_buf, RX_SIZE);
                    string time_recvmessage(RX_buf,RX_buf+ret);
                    cout<<time_recvmessage<<endl;
                    string recv_protocal =time_recvmessage.substr(1, 2);
                    cout<<recv_protocal<<endl;
                    string recv_mac = time_recvmessage.substr(16, 16);
                    cout<<recv_mac<<endl;
                    if(recv_protocal==TIME_SEND && recv_mac==mac )
                    {
                        current_time = time_recvmessage.substr(32, 17);   //获取网关下发参数时间戳 2016 08 01 08 58 57 223
                        string year_time_info = current_time.substr(0, 4);
                        string month_time_info = current_time.substr(4, 2);
                        string day_time_info = current_time.substr(6, 2);
                        string hour_time_info = current_time.substr(8, 2);
                        string minute_time_info = current_time.substr(10, 2);
                        string sec_time_info = current_time.substr(12, 2);
                        string msec_time_info = current_time.substr(14, 3);
                        char time[50] = "";
                        sprintf(time, "date %s-%s-%s", year_time_info.c_str(), month_time_info.c_str(), day_time_info.c_str());
                        cout << "current year :" << time << endl;
                        system(time);
                        memset(time, 0, sizeof(time));
                        sprintf(time, "date %s:%s:%s", hour_time_info.c_str(), minute_time_info.c_str(), sec_time_info.c_str());
                        cout << "current hour :" << time << endl;
                        system(time); 
                        break;
                    }
                }  
            }
#endif
        }
        else
        {
            select_num = 1; 
            goto select_again;
        }
    }
}

/**
*   GPS数据处理
*   @param *line 需要处理得数据
*   @param *lat  纬度
*   @param *flag_lat  标志 E
*   @param *lon  经度
*   @param *flag_lon  标志 N
*   @return  -1 失败  0 成功
*
*/
int process_gps(char *line, char *lat, char *flag_lat, char *lon, char *flag_lon)
{
    char *token;
    token = strstr(line, "V");
    if (token != NULL)
    {
        return -1;
    }
    token = strtok(line, ",");
    if (token != NULL && !strcmp(token, "$GNRMC"))
    {
        char *time = strtok(NULL, ",");
        char *a = strtok(NULL, ",");
        char *nd = strtok(NULL, ",");
        char *n = strtok(NULL, ",");
        char *ed = strtok(NULL, ",");
        char *e = strtok(NULL, ",");
        strcpy(lat, nd);
        strcpy(flag_lat, n);
        strcpy(lon, ed);
        strcpy(flag_lon, e);
    }
    else {
        return -1;

    }
    return 0;
}

/**
*获取南水所的数据
*/
void* Get_L_data(void* arg){
    char rj45_0Data[1500] = {0};  //用于保存获取的数据
    fd_set rest,allset; 
    int client[256];//是为了保存已经建立连接并返回的新套接字描述符
    int i,maxi,maxfd,confd,sockfd;
    int client_num=0;
    //char buf[1024];
    FD_ZERO(&allset);
    FD_SET(fdL_lan,&allset);
    socklen_t cliaddr_len;  //套接字地址的长度
    ssize_t n;  //在进行文件读写操作时返回写入或读取的字节数
    for(i=0;i<64;i++){
        client[i]=-1;
    }
    maxfd=fdL_lan;
    maxi=-1;
    char temp[1500]={0};
    cout<<"--------Waiting for terminal connection--------"<<endl;
    while(1){
        rest=allset;
        int ret=select(maxfd+1,&rest,NULL,NULL,NULL);
        if(ret<0){
            printf("select error");
            //break;
        } //有客户端连接
        if(FD_ISSET(fdL_lan,&rest)){
             cliaddr_len=sizeof(Llan_cli_addr);
             confd=accept(fdL_lan,(struct sockaddr*)&Llan_cli_addr,&cliaddr_len);  //返回已连接的socket描述字
             client_num++;//连接的客户端数加1
             cout<<"--------Connect successfully--------"<<endl;
             cout<<"-------File descriptor："<<confd<<"--------"<<endl;
             for(i=0;i<64;i++){
                if(client[i]<0){
                    client[i]=confd;
                    break;
                }
             }
            FD_SET(confd,&allset);
            if(confd>maxfd) maxfd=confd;
            if(i>maxi) maxi=i;
            if(--ret==0) continue; 
        }

       
        for(i=0;i<=maxi;i++){   //监测是否有建立连接的客户端发送数据
            if((sockfd=client[i])<0) continue;
            if(FD_ISSET(sockfd,&rest)){
                if(( n=read(sockfd,rj45_0Data,1500))==0){
                    client_num--;
                    cout<<"--------Client quit--------"<<endl;
                    cout<<"--------Number of current client--------:"<<client_num<<endl; 
                    close(sockfd);           // 当client关闭链接时，服务器端也关闭对应链接 
                    FD_CLR(sockfd, &allset); // 解除select监控此文件描述符 
                    memset(rj45_0Data,0,sizeof(rj45_0Data));
                    client[i] = -1;
                    //break;

                }else{
#ifdef DEBUG
                cout<<"--------Data of RJ45 listening--------："<<rj45_0Data<<endl;
#endif              
                cout<<"--------Number of current client--------:"<<client_num<<endl; 
                string dataRj45 = packet06(string(rj45_0Data), RJ45);  //封装为私有协议06包
                transMessage.push(dataRj45);                     //将封装好的数据包放到发送队列
                write(sockfd,rj45_0Data,n);  //回显，可关闭回显
                memset(rj45_0Data,0,sizeof(rj45_0Data));
                }
                if(--ret==0) break;
            }
        }
    }
    close(fdL_lan);
}
/**
*   GPS数据获取
*   @return
*
*/
void* GET_GPS(void* arg)
{
    // cout << "================================" << endl;
    //首先开启gos获取脚本
#ifdef DEBUG
    cout << "Start thread：GET_GPS()" << endl;
#endif
    system("/home/root/gps_test.sh &");
    sleep(60);
    int ret;
    uint8_t dev[] = "/dev/ttymxc7";   //gps设备描述符
    char buf[300];
    char f_lat[5];
    char f_lon[5];
    char lat[10];
    char lon[10];
    int fd;         //接收GPS数据的串口
    int i = 0;
    fd = open((const char*)dev, O_RDWR | O_NOCTTY); // O_NOCTTY不将该文件作为进程的控制终端
    if (-1 == fd)
    {
#ifdef DEBUG
        cout << "Open GPS device error!" << endl;
#endif
    }
    else {
#ifdef DEBUG
        cout << "Open GPS device success!" << endl;
#endif
        ret = set_opt(fd, 9600, 8, 'n', 1);
        int flag = 0;
        int count = 1;
        char in;
        int i = 0;
        int flag_read = 0;
        while (1)
        {
            while (1)
            {
                if (read(fd, &in, 1) == 0)break;
                if (in != '$')
                {
                    if (0 == flag_read)continue;
                    else if (in == '\n' || in == '*')
                    {
                        flag_read = 0;
                        i = 0;
                        break;
                    }
                    else if (flag_read = 1 && in != '\n')buf[i++] = in;
                }
                else if (in == '$' && flag_read == 0)
                {
                    flag_read = 1;
                    i = 0;
                    buf[i++] = in;
                }
            }
#ifdef DEBUG
            //cout << "GPS: " << buf << endl;
#endif 
            flag = process_gps(buf, lat, f_lat, lon, f_lon);
            if (-1 == flag)
            {
#ifdef DEBUG
                //       cout << "GPS analyze error" << endl;
#endif          

                //N2932,E10636
                // mutex_gps.lock();
                // gps_lat = "NFFFF";   //纬度 北纬
                // gps_lon = "EFFFFF";  //经度   东经
                // mutex_gps.unlock();
                continue;

            }
            else {
#ifdef DEBUG
                //cout << "lat: " << lat << " lon: " << lon << " count: " << count << endl;
                //printf("%c%c度%c%c分%s %c%c%c度%c%c分%s\n", lat[0], lat[1], lat[2], lat[3], f_lat, lon[0], lon[1], lon[2], lon[3], lon[4], f_lon);
#endif
                //lon是经度 lat是纬度
                string s_lon = "";
                s_lon = s_lon + f_lon + lon[0] + lon[1] + lon[2] + lon[3] + lon[4]; //经度
                string s_lat = "";
                s_lat = s_lat + f_lat + lat[0] + lat[1] + lat[2] + lat[3];   //纬度

                mutex_gps.lock();
                gps_lat = s_lat;
                gps_lon = s_lon;
                mutex_gps.unlock();

            }
            sleep(2);
        }
    }
}





// string i2hex(int num)
// {
//     //将int转换为hex
//     string out = "";
//     char h = '0', l = '0';
//     h = '0' + (num / 16);
//     l = '0' + (num - (num / 16) * 16);
//     out = out + h + l;
//     return out;
// }
// string buffer [33]; //用于存放转换好的十六进制字符串，可根据需要定义长度
// char * inttohex(int aa)
// {
//     static int i = 0;

//     if (aa < 16)            //递归结束条件 
//     {
//         if (aa < 10)        //当前数转换成字符放入字符串 
//         buffer[i] = aa + '0';
//         else
//         buffer[i] = aa - 10 + 'A';
//         buffer[i+1] = '\0'; //字符串结束标志 
//     }
//     else
//     {
//         inttohex(aa / 16);  //递归调用 
//         i++;                //字符串索引+1 
//         aa %= 16;           //计算当前值
//         if (aa < 10)        //当前数转换成字符放入字符串 
//         buffer[i] = aa + '0';
//         else
//         buffer[i] = aa - 10 + 'A';
//     }
//         return (buffer);
// }

/**
*   将int型数据转换成特定格式的字符串，比如：1转换成“01”，11转换成“11”
*   @param source 需要转换的数据
*   @param *dest  目标字符串
*   @return
*
*/
void string_formater(int source, char *dest)
{
    if (source < 10)
    {
        char  temp[3];
        strcpy(dest, "0");
        sprintf(temp, "%d", source);
        strcat(dest, temp);
    }
    else
        sprintf(dest, "%d", source);
}



/**
*   将int型数据转换成特定格式的字符串，比如：1转换成“001”，11转换成“011”，111转换成“111”
*   @param source 需要转换的数据
*   @param *dest  目标字符串
*   @return
*
*/
void string_formater_usec(int source, char *dest)
{
    if (source < 10)
    {
        char  temp[3];
        strcpy(dest, "00");
        sprintf(temp, "%d", source);
        strcat(dest, temp);
    }
    else if (source < 100)
    {
        char  temp[3];
        strcpy(dest, "0");
        sprintf(temp, "%d", source);
        strcat(dest, temp);

    }
    else
        sprintf(dest, "%d", source);
}



// *
// *   获取系统当前时间
// *   @return  返回字符串时间
// *

string time_now_to_string()
{
    struct timeval  tv;
    struct tm  *p;
    char timestamp_now[100];
    char stringyear[5], stringmonth[3], stringday[3], stringhour[3], stringmin[3], stringsec[3], stringusec[4];

    gettimeofday(&tv, NULL);
    p = localtime(&tv.tv_sec);
    sprintf(stringyear, "%d", (p->tm_year + 1900));
    string_formater(p->tm_mon + 1, stringmonth);
    string_formater(p->tm_mday, stringday);
    string_formater(p->tm_hour, stringhour);
    string_formater(p->tm_min, stringmin);
    string_formater(p->tm_sec, stringsec);
    string_formater_usec(tv.tv_usec / 1000, stringusec);

    strcpy(timestamp_now, stringyear);
    strcat(timestamp_now, stringmonth);
    strcat(timestamp_now, stringday);
    strcat(timestamp_now, stringhour);
    strcat(timestamp_now, stringmin);
    strcat(timestamp_now, stringsec);
    strcat(timestamp_now, stringusec);

    string out = "";
    out = out + timestamp_now;
    return out;
}



/**
*   CRC16循环冗余校验算法
*   @param *puchMsg 需要校验的字符串指针
*   @param usDataLen 要校验的字符串长度
*   @return  返回 CRC16 校验码
*
*/
unsigned int CRC16_Checkout(unsigned char *puchMsg, unsigned int usDataLen)
{
    unsigned int i, j, crc_reg, check;
    crc_reg = 0xFFFF;
    for (i = 0; i < usDataLen; i++)
    {
        crc_reg = (crc_reg >> 8) ^ puchMsg[i];
        for (j = 0; j < 8; j++)
        {
            check = crc_reg & 0x0001;
            crc_reg >>= 1;
            if (check == 0x0001)
            {
                crc_reg ^= 0xA001;
            }
        }
    }
    return crc_reg;
}
/**********************************无人船相关API**************************************************************************/
/**
*共用体获取32位精度浮点数
*/
union Tdata
{
    float testData_float;
    unsigned char testArray[4];
}TData;

/*
*利用函数获取32位精度浮点数
*/
float float2decimal(long int byte0, long int byte1, long int byte2, long int byte3)
{

    long int realbyte0, realbyte1, realbyte2, realbyte3;
    char S;
    long int E, M;
    float D;
    realbyte0 = byte3; 
    realbyte1 = byte2; 
    realbyte2 = byte1; 
    realbyte3 = byte0;
    if ((realbyte0 & 0x80) == 0) {
        S = 0;//正数
    }
    else {
        S = 1;
    }
    E = ((realbyte0 << 1) | (realbyte1&0x80) >> 7) - 127;
    M = ((realbyte1 & 0x7f) << 16) |(realbyte2 << 8) | realbyte3;
    D = pow(-1, S)*(1.0 + M / pow(2, 23))* pow(2, E);
    return D;
}
/*
*返回一个字的高8位
*/
uint8_t int16_t_getH(uint16_t a) 
{    //返回一个字的高8位
    uint8_t aa = (a>> 8)&0xFF;
    return aa;
}
/*
*返回一个字的低8位
*/
uint8_t int16_t_getL(uint16_t a) 
{    //返回一个字的低8位
    uint8_t b = 0xff & a;
    return b;
}
/*
获取无人船传感器数据
*/
void* get_ship_data(void *arg)
{   
    /*注册软件看门狗*/
    CSoftwareWdt *g_CsoftwareWdt=(CSoftwareWdt *)arg;
    const char* const threadname="get_ship_data";
    wdtShipId = g_CsoftwareWdt->RequestSoftwareWdtID(threadname,softdogTimeout);//线程注册

    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    int slave_address[11] = { 1,2,3,4,5,6,7,8,10,11,12}; //COD、蓝绿素、浊度、测深仪流速仪、MH-SX超声波测深、气象站
    modbus_t * ctx1=NULL;
    //uint8_t 保存数据
    uint16_t tab_reg[512] = { 0 };
    uint8_t temp_data[512] = {};
    uint8_t ship_getpacket[2048]={0};
    float sensor_data[12] = {};
    float temper,yels,ph,diandl,rongO,NH,cod,toc,lanlz,zhuod,lius,shend;
    /* 风向，风速，2分钟风速，10分钟风速，环境温度，最高温度，最低温度，环境湿度，露点温度，气压，光照度，雨量*/
    //float fengx,fengs,fengs_2,fengs_10,temper_en,temper_max,temper_min,humid_en,humid_expose,qiy,guangzd,yul;
    float fengs,fengs_2,fengs_10,temper_en,temper_max,temper_min,humid_en,humid_expose,qiy,yul;
    int fengx,guangzd;
    int regs = 0;
    int index_flag = 0;
    /*数据前缀编码*/
    char sensor_ph_code[] = "w01001";
    char sensor_rongO_code[] ="w01009";
    char sensor_temper_code[] = "w01010";            
    char sensor_diand_code[] = "w01014";
    char sensor_NH_code[] = "w21003";
    char sensor_cod_code[] = "w00001";
    char sensor_toc_code[] = "w00002";
    char sensor_lanlz_code[] = "w00003";
    char sensor_yels_code[] = "w00004";
    char sensor_zhuod_code[] = "w00005";
    char sensor_lius_code[] = "w00006";
    char sensor_shend_code[] = "w00007";
    /*气象仪数据前缀编码*/
    char sensor_fengx_code[] = "q00001";
    char sensor_fengs_code[] ="q00002";
    char sensor_fengs_2_code[] = "q00003";            
    char sensor_fengs_10_code[] = "q00004";
    char sensor_temper_en_code[] = "q00005";
    char sensor_temper_max_code[] = "q00006";
    char sensor_temper_min_code[] = "q00007";
    char sensor_humid_en_code[] = "q00008";
    char sensor_humid_expose_code[] = "q00009";
    char sensor_qiya_code[] = "q00010";
    char sensor_guangzd_code[] = "q00011";
    char sensor_yul_code[] = "q00012";
    
    /*数据段结构组成表*/
    //请求编码QN*20
    string hj_QN = "";  //时间
    //hj_QN+=time_now_to_string();
    //系统编码*5 海水质量监测
    char hj_ST[] = "26";   
    //命令编码*7
    char hj_CN[] = "2011";  
    //访问密码*9
    char hj_PW[] = "123456";
    //设备唯一标识符*27
    string hj_MN =get_MN();
    //拆分包及应答标志*8
    char hj_Flag[] = "4";
    //string accessdev_gps = "N2932E10636"; 
    string accessdev_gps = gps_lat + gps_lon;   
    //RTU
    ctx1 = modbus_new_rtu("/dev/ttymxc4", 9600, 'N', 8, 1);
    modbus_rtu_set_serial_mode(ctx1, MODBUS_RTU_RS485);
    modbus_rtu_set_rts(ctx1, MODBUS_RTU_RTS_DOWN);
    modbus_set_debug(ctx1, TRUE);
    //建立连接
    if (modbus_connect(ctx1) == -1) {
        printf("connection failed");
        modbus_strerror(errno);
        modbus_free(ctx1);
        return false;
    }
    float tempfloat1, tempfloat2,tempfloat3;  //读4字节单精度浮点数
    while (1) {
        cout<<"获取无人船传感器数据"<<endl;
        //一直更新GPS信息数据
        accessdev_gps = gps_lat + gps_lon;   
        //memset(tab_reg, 0, sizeof(tab_reg));
        modbus_set_slave(ctx1, slave_address[index_flag]); //设置从机地址
        memset(tab_reg, 0, sizeof(tab_reg));
        //开始读取数据
        switch (index_flag) {
        case 0: //从设备1叶绿素
            regs = modbus_read_registers(ctx1, 9728, 4, tab_reg); //对应于读取温度值、叶绿素值    d c a b 
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a=0;
                b=0;
                c=0;
                d=0;
            }
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            temper = float2decimal(a, b, c, d);
            sensor_data[0] = temper; //保存温度
            printf("%f\n", temper);

            a = int16_t_getH(tab_reg[2]);
            b = int16_t_getL(tab_reg[2]);
            c = int16_t_getH(tab_reg[3]);
            d = int16_t_getL(tab_reg[3]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            yels = float2decimal(a, b, c, d);
            sensor_data[1] = yels;  //保存叶绿素 
            printf("%f\n", yels);
            sleep(2);
            break;
        case 1: //从设备2 PH传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取PH值 02 03 00 01 00 02 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a=0;
                b=0;
                c=0;
                d=0;
            }
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            TData.testArray[3] = c;
            TData.testArray[2] = d;
            TData.testArray[1] = a;
            TData.testArray[0] = b;
            ph = TData.testData_float;
            sensor_data[2] = ph; //保存PH
            sleep(2);
            break;
        case 2: //从设备3 电导率传感器
            regs = modbus_read_registers(ctx1, 0, 2, tab_reg); //对应于读取电导率值 03 03 00 00 00 02 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a=0;
                b=0;
                c=0;
                d=0;
            }
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            TData.testArray[3] = c;
            TData.testArray[2] = d;
            TData.testArray[1] = a;
            TData.testArray[0] = b;
            diandl = TData.testData_float;
            sensor_data[3] = diandl; //保存电导率
            sleep(2);
            break;
        case 3: //从设备4 溶解氧传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取溶解氧值 04 03 00 01 00 02 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a=0;
                b=0;
                c=0;
                d=0;
            }
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            TData.testArray[3] = c;
            TData.testArray[2] = d;
            TData.testArray[1] = a;
            TData.testArray[0] = b;
            rongO = TData.testData_float;
            sensor_data[4] = rongO; //保存溶解氧
            sleep(2);
            break;
        case 4: //从设备5 氨氮传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取氨氮值 05 03 00 01 00 02 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a=0;
                b=0;
                c=0;
                d=0;
            }
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            TData.testArray[3]=c;
            TData.testArray[2]=d;
            TData.testArray[1]=a;
            TData.testArray[0]=b;
            NH = TData.testData_float;
            sensor_data[5] = NH;  //保存氨氮
            sleep(2);
            break;
        case 5://从设备2 COD传感器
            regs = modbus_read_registers(ctx1, 9728, 6, tab_reg); //对应于读取温度值、COD值、TOC值
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            tempfloat1 = float2decimal(a, b, c, d);
            sensor_data[6] = tempfloat1; //保存温度


            a = int16_t_getH(tab_reg[2]);
            b = int16_t_getL(tab_reg[2]);
            c = int16_t_getH(tab_reg[3]);
            d = int16_t_getL(tab_reg[3]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            cod = float2decimal(a, b, c, d);
            sensor_data[7] = cod;  //保存COD
        

            a = int16_t_getH(tab_reg[4]);
            b = int16_t_getL(tab_reg[4]);
            c = int16_t_getH(tab_reg[5]);
            d = int16_t_getL(tab_reg[5]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            toc = float2decimal(a, b, c, d);
            sensor_data[8] = toc; //保存TOC
             sleep(2);
            break;
        case 6: //从设备7 蓝绿藻传感器
            regs = modbus_read_registers(ctx1, 9728, 4, tab_reg); //对应于读取温度蓝绿藻值 07 03 26 00 00 04 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a=0;
                b=0;
                c=0;
                d=0;
            }
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            tempfloat1 = float2decimal(a, b, c, d);
            printf("温度%f", tempfloat1);
            sensor_data[9] = tempfloat1; //保存温度
            printf("温度2%f", sensor_data[9]);

            a = int16_t_getH(tab_reg[2]);
            b = int16_t_getL(tab_reg[2]);
            c = int16_t_getH(tab_reg[3]);
            d = int16_t_getL(tab_reg[3]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            lanlz = float2decimal(a, b, c, d);
            sensor_data[10] = lanlz; //保存蓝绿值
            sleep(2);
            break;
        case 7: //从设备8 浊度传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取浊度值 08 03 00 01 00 02 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a=0;
                b=0;
                c=0;
                d=0;
            }
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            TData.testArray[3] = c;
            TData.testArray[2] = d;
            TData.testArray[1] = a;
            TData.testArray[0] = b;
            zhuod = TData.testData_float;
            sensor_data[11] = zhuod; //保存浊度值
            sleep(2);
            break;
        case 8: //从设备10 超声波流速仪
            regs = modbus_read_input_registers(ctx1, 2, 2, tab_reg);
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            TData.testArray[3] = a;
            TData.testArray[2] = b;
            TData.testArray[1] = c;
            TData.testArray[0] = d;
            lius=TData.testData_float;;
            //lius=5.645;
            sleep(2);
            break;
        case 9: //从设备11 超声波测深仪
            regs = modbus_read_input_registers(ctx1, 0, 2, tab_reg);
            a = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[1]);
            d = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            TData.testArray[3] = a;
            TData.testArray[2] = b;
            TData.testArray[1] = c;
            TData.testArray[0] = d;
            shend=TData.testData_float;;
            sleep(2);
            break;
        case 10://从设备12 气象站
            regs = modbus_read_registers(ctx1, 0, 12, tab_reg); //对应于读取浊度值 08 03 00 01 00 02 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false; 
                a=0;
                b=0;
                c=0;
                d=0;
            }
            //风向，风速，2分钟风速，10分钟风速，环境温度，最高温度，最低温度，环境湿度，露点湿度，气压，光照度，雨量*/
            /*fengx=16;fengs=3.2;fengs_2=4.2;fengs_10=2.8;
            temper_en=24.5;temper_max=30.0;temper_min=18.0;
            humid_en=75.3;humid_expose=80.1;qiy=1013.25;guangzd=12;yul=50.4;*/
            fengx=tab_reg[0];fengs=int2pString(tab_reg[1]);fengs_2=int2pString(tab_reg[2]);fengs_10=int2pString(tab_reg[3]);
            temper_en=int2pString(tab_reg[4]);temper_max=int2pString(tab_reg[5]);temper_min=int2pString(tab_reg[6]);humid_en=int2pString(tab_reg[7]);
            humid_expose=int2pString(tab_reg[8]);qiy=int2pString(tab_reg[9]);guangzd=tab_reg[10];yul=int2pString(tab_reg[11]);

            sleep(2);
            break;
        }        
        if (index_flag == 10) {
            index_flag = -1;
            hj_QN = time_now_to_string(); //一次采集完毕获取时间
            printf("温度值：%f\n叶绿素值：%f\nPH:%f\n电导率：%f\n溶解氧：%f\n氨氮：%f\n温度：%f\nCOD:%f\nTOC:%f\n温度：%f\n蓝绿藻:%f\n浊度:%f\n流速:%f\n深度:%f\n风向:%d\n风速:%f\n2分钟风速:%f\n10分钟风速:%f\n环境温度:%f\n最高温度:%f\n最低温度:%f\n环境湿度:%f\n露点湿度:%f\n气压:%f\n光照度:%d\n雨量:%f\n",
                sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3], sensor_data[4], sensor_data[5], sensor_data[6], sensor_data[7]
                , sensor_data[8], sensor_data[9], sensor_data[10], sensor_data[11],lius,shend,fengx,fengs,fengs_2,fengs_10,temper_en,temper_max,temper_min,humid_en,humid_expose,qiy,guangzd,yul);
            /* 原生数据的封装 */
            printf("test01\n");
            sprintf((char*)ship_getpacket,"QN=%s;ST=%s;CN=%s;PW=%s;MN=%s;Flag=%s;CP=&&Gps=%s;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010d;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010.3lf;%s-Rtd=%010d;%s-Rtd=%010.3lf&&",\
                hj_QN.c_str(), hj_ST,hj_CN, hj_PW, hj_MN.c_str(),hj_Flag,accessdev_gps.c_str(),sensor_temper_code, temper, sensor_yels_code, yels,\
                sensor_ph_code, ph,sensor_diand_code, diandl, sensor_rongO_code, rongO, sensor_NH_code, NH, \
                sensor_cod_code, cod, sensor_toc_code, toc, sensor_lanlz_code, lanlz, sensor_zhuod_code, zhuod,sensor_lius_code,lius,sensor_shend_code,shend,sensor_fengx_code,fengx,sensor_fengs_code,fengs,sensor_fengs_2_code,fengs_2,sensor_fengs_10_code,\
                fengs_10,sensor_temper_en_code,temper_en,sensor_temper_max_code,temper_max,sensor_temper_min_code,temper_min,sensor_humid_en_code,humid_en,\
                sensor_humid_expose_code,humid_expose,sensor_qiya_code,qiy,sensor_guangzd_code,guangzd,sensor_yul_code,yul);
            printf("test02\n");
            cout<<"无人船传感器数据："<<ship_getpacket<<endl;
            /*确保数据长度为4位*/
            int len_origin=strlen((char*)ship_getpacket);           
            string data_len=ensureLen_4(len_origin);
            /*进行循环冗余码的计算并保证为4位并完成HJ212的数据包封装*/
            unsigned int hjcrc_ship=CRC16_Checkout(ship_getpacket,len_origin);
            string hj212pacet=ensure_crc4_packet(hjcrc_ship,data_len,ship_getpacket);
            string waterData06 = packet06(hj212pacet, RS485_A);
            transMessage.push(waterData06);
            //shipsensor_message.push(hj212pacet);
            cout<<"无人船传感器HJ212数据包:"<<hj212pacet<<endl;
            /*进行私有协议封包看后续情况*/
            /*进行私有协议封包看后续情况*/

            /*利用锁机制和条件变量进行线程控制*/
            int S=pthread_mutex_lock(&mtx_sensor);
            if(S<0) perror("pthread_mutex_lock()");
            while(iS_getshipsensor==0){  //检查是否收到了03请求消息命令
            pthread_cond_wait(&cond_sensor,&mtx_sensor);  //如果没有在这里阻塞等待ISR给SAP下03命令,防止cpu资源浪费
            }
            while(iS_getshipsensor>0){    
            iS_getshipsensor=0;
            }
            pthread_mutex_unlock(&mtx_sensor);           
        }
        index_flag++;
        g_CsoftwareWdt->KeepSoftwareWdtAlive(wdtShipId); //软件喂狗
        printf("thread of sensor in ship feed dog success\n");
    }
}
/*******************************无人船相关API********************************************************************************/
void* get_sensor_data(void *arg)
{

    cout << "Thread: start get_airsensor_data" << endl;
    CSoftwareWdt *g_CsoftwareWdt=(CSoftwareWdt *)arg;
    const char*  threadname="get_sensor_data";
    int wdt_id=-1;
    wdt_id=g_CsoftwareWdt->RequestSoftwareWdtID(threadname,softdogTimeout + 10);//当前此线程注册
    int regs = 0;
    int index = 0;
    uint16_t tab_reg[64] = { 0 };
    string cpu_mem="";
    modbus_t *ctx = NULL;
    int S;


    //打开端口：端口，波特率、校验位、数据位、停止位
    ctx = modbus_new_rtu(MODBUS_TTY, BAUD_RATE, 'N', 8, 1);
    //设置从机地址
    modbus_set_slave(ctx, SLAVE_ADDR);
    //设置串口模式（可选）
    modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    //设置RTS（可选）
    modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_DOWN);
    //建立连接
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed:%s\n", modbus_strerror(errno));
        modbus_free(ctx);
        //return -1;
    }
    //设置应答延时（可选）
    modbus_set_response_timeout(ctx, 0, 1000000);
    uint8_t temp_data_str[512] = {};
    int sensor_so2work, sensor_so2au, sensor_ke25, sensor_tgs822, sensor_tgs813, sensor_tgs2602, sensor_tgs2620;
    int sensor_mq131, sensor_nob4work, sensor_nob4au, sensor_nh3b1work, sensor_nh3b1au, sensor_no2b43fwork, sensor_no2b43fau, sensor_h2sb4work, sensor_h2sb4au;
    double sensor_t6703, sensor_jxmco, sensor_temper, sensor_humid, sensor_pm25, sensor_pm10;
    while (1)  //一直读取数据
    {
        g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
        cout<<"thread of Airsensor feed dog success"<<endl;

        memset(temp_data_str, 0, sizeof(temp_data_str));

        //获取传感器信息
        index = 0;
        memset(tab_reg, 0, sizeof(tab_reg));
        regs = modbus_read_registers(ctx, 0, 44, tab_reg);
        if (regs == -1)
        {
            fprintf(stderr, "%s\n", modbus_strerror(errno));
            // return -1;
            printf("read registers error\n");
            continue;
        }



        sensor_so2work = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_so2au = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_ke25 = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_tgs822 = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_tgs813 = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_tgs2602 = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_tgs2620 = (tab_reg[index++]) | tab_reg[index++] << 16;

        sensor_mq131 = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_nob4work = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_nob4au = (tab_reg[index++]) | tab_reg[index++] << 16;

        sensor_nh3b1work = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_nh3b1au = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_no2b43fwork = (tab_reg[index++]) | tab_reg[index++] << 16;

        sensor_no2b43fau = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_h2sb4work = (tab_reg[index++]) | tab_reg[index++] << 16;
        sensor_h2sb4au = (tab_reg[index++]) | tab_reg[index++] << 16;

        sensor_t6703 = ((double)((tab_reg[index++]) | tab_reg[index++] << 16)) / 100;
        sensor_jxmco = ((double)((tab_reg[index++]) | tab_reg[index++] << 16)) / 100000;
        sensor_temper = ((double)((tab_reg[index++]) | tab_reg[index++] << 16)) / 10;
        sensor_humid = ((double)((tab_reg[index++]) | tab_reg[index++] << 16)) / 10;
        sensor_pm25 = ((double)((tab_reg[index++]) | tab_reg[index++] << 16)) / 100;
        sensor_pm10 = ((double)((tab_reg[index++]) | tab_reg[index++] << 16)) / 100;

        /**hj212封包**/
        /*通讯报结构组成*/

        //数据长度*4
        char hj_len[] = "";
        //CRC校验*4
        



        /*数据段结构组成表*/
        //请求编码QN*20
        string hj_QN = "";  //时间
        //系统编码*5
        char hj_ST[] = "31";  
        //命令编码*7
        char hj_CN[] = "2011";  
        //访问密码*9
        char hj_PW[] = "123456";
        //设备唯一标识符*27
        string hj_MN = "";
        //拆分包及应答标志*8
        char hj_Flag[] = "4";  //不分包
                               // //总包数*9     
                               // char hj_PNUM[] = "";
                               // //包号*8
                               // char hj_PNO[] = "";
                               //指令参数CP*n(0=<n<=950)
                               // char hj_CP[] = "";


        char sensor_so2_code[] = "a21026";
        char sensor_o2_code[] = "a19001";
        char sensor_ch4_code[] = "a05002";               //tgs813
        char sensor_no_code[] = "a21003";
        char sensor_nh3_code[] = "a21001";
        char sensor_no2_code[] = "a21004";
        char sensor_h2s_code[] = "a21028";
        char sensor_co2_code[] = "a05001";
        char sensor_co_code[] = "a21005";
        char sensor_temper_code[] = "a01001";
        char sensor_humid_code[] = "a01002";
        char sensor_pm25_code[] = "a34004";
        char sensor_pm10_code[] = "a34002";


        //自定义编码字段
        //char accessdev_gps_code[] = "d02000";
        //char accessdev__netnum_code[] = "d02001";
        //char accessdev_idnum_code[] = "d02002";

        //lon是经度 lat是纬度
        //N是北纬 e是东经

        // string accessdev_gps="";
        // accessdev_gps = gps_lat +  "," + gps_lon ;
        string accessdev_gps = gps_lat + gps_lon;
        cpu_mem=get_cpuOccupy();
        //accessdev_gps = "N2932E10636";

        // cout << "gps_lat:"<<gps_lat<<endl;
        // cout << "gps_lon:"<<gps_lon<<endl;





        // printf("====================打印传感数据=============================\n");

        // printf("[01][%d] (0-65535) SO2_B4-WORK(Sulfur dioxide)\n", sensor_so2work);//二氧化硫
        // printf("[02][%d] (0-65535) SO2_B4-AU(Sulfur dioxide)\n", sensor_so2au);//二氧化硫
        // printf("[03][%d] (0-65535) KE_25(oxygen)\n", sensor_ke25);//氧气
        // printf("[04][%d] (0-65535) TGS822(Organic solvents)\n", sensor_tgs822);//有机溶剂
        // printf("[05][%d] (0-65535) TGS813(flammable gas)\n", sensor_tgs813);//可燃气体
        // printf("[06][%d] (0-65535) TGS2602(VOC)\n", sensor_tgs2602);//挥发性有机物
        // printf("[07][%d] (0-65535) TGS2620(alcohol)\n", sensor_tgs2620);//酒精
        // printf("[08][%d] (0-65535) MQ131(ozone) \n", sensor_mq131);//臭氧
        // printf("[09][%d] (0-65535) NO_B4-WORK(Nitric oxide)\n", sensor_nob4work);//一氧化氮
        // printf("[10][%d] (0-65535) NO_B4-AU(Nitric oxide)\n", sensor_nob4au);//一氧化氮
        // printf("[11][%d] (0-65535) NH3_B1-WORK(Ammonia)\n", sensor_nh3b1work);//氨气
        // printf("[12][%d] (0-65535) NH3_B1-AU(Ammonia)\n", sensor_nh3b1au);//氨气
        // printf("[13][%d] (0-65535) NO2_B43F-WORK(Nitrogen Dioxide)\n", sensor_no2b43fwork);//二氧化氮
        // printf("[14][%d] (0-65535) NO2_B43F-AU(Nitrogen Dioxide)\n", sensor_no2b43fau);//二氧化氮
        // printf("[15][%d] (0-65535) H2S_B4-WORK(Hydrogen sulfide)\n", sensor_h2sb4work);//硫化氢
        // printf("[16][%d] (0-65535) H2S_B4-AU(Hydrogen sulfide)\n", sensor_h2sb4au);//硫化氢
        // printf("[17][%lf] (0.01-5000ppm) T6703(carbon dioxide)\n", sensor_t6703);//二氧化碳
        // printf("[18][%lf] (0.01-1000ppm) JXM_CO(Carbon monoxide)\n", sensor_jxmco);//一氧化碳
        // printf("[19][%lf] (0.1C) SHT20(temperature)\n", sensor_temper);//温度
        // printf("[20][%lf] (0.1%%) SHT20(humidity)\n", sensor_humid);//湿度
        // printf("[21][%lf] (0.01-1000ppm) PM_D4(pm2.5)\n", sensor_pm25);//pm2.5
        // printf("[22][%lf] (0.01-1000ppm) PM_D4(pm10)\n", sensor_pm10);//pm10 


        hj_MN = get_MN();



        hj_QN = hj_QN + time_now_to_string();


        //*****************************************************封装HJ212数据段，原生数据***************************************************
        sprintf((char*)temp_data_str, "QN=%s;ST=%s;CN=%s;PW=%s;MN=%s;Flag=%s;CP=&&Gps=%s;%s-Rtd=%08d;%s-Rtd=%08d;%s-Rtd=%08d;%s-Rtd=%08d;%s-Rtd=%08d;%s-Rtd=%08d;%s-Rtd=%08d;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.1lf;%s-Rtd=%08.1lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf&&",\
         hj_QN.c_str(), hj_ST, hj_CN, hj_PW, hj_MN.c_str(), \
            hj_Flag,accessdev_gps.c_str(), sensor_so2_code, sensor_so2work, sensor_o2_code, sensor_ke25, sensor_ch4_code, sensor_tgs813, \
            sensor_no_code, sensor_nob4work, sensor_nh3_code, sensor_nh3b1work, sensor_no2_code, sensor_no2b43fwork, \
            sensor_h2s_code, sensor_h2sb4work, sensor_co2_code, sensor_t6703, sensor_co_code, sensor_jxmco, sensor_temper_code, \
            sensor_temper, sensor_humid_code, sensor_humid, sensor_pm25_code, sensor_pm25, sensor_pm10_code, sensor_pm10);

            
        //数据段
        // sprintf((char*)temp_data_str, "QN=%s;ST=%s;CN=%s;PW=%s;MN=%s;Flag=%s;CP=&&%s-Rtd=%d;%s-Rtd=%d;%s-Rtd=%d;%s-Rtd=%d;%s-Rtd=%d;%s-Rtd=%d;%s-Rtd=%d;%s-Rtd=%0.1lf;%s-Rtd=%0.1lf;%s-Rtd=%0.1lf;%s-Rtd=%0.1lf;%s-Rtd=%0.1lf;%s-Rtd=%0.1lf;%s=%s;%s=%s;%s=%s&&", hj_QN.c_str(), hj_ST, hj_CN, hj_PW, hj_MN.c_str(), \
        //     hj_Flag, sensor_so2_code, sensor_so2work, sensor_o2_code, sensor_ke25, sensor_ch4_code, sensor_tgs813, \
        //     sensor_no_code, sensor_nob4work, sensor_nh3_code, sensor_nh3b1work, sensor_no2_code, sensor_no2b43fwork, \
        //     sensor_h2s_code, sensor_h2sb4work, sensor_co2_code, sensor_t6703, sensor_co_code, sensor_jxmco, sensor_temper_code, \
        //     sensor_temper, sensor_humid_code, sensor_humid, sensor_pm25_code, sensor_pm25, sensor_pm10_code, sensor_pm10, accessdev_gps_code, accessdev_gps.c_str(), \
        //     accessdev__netnum_code, net_id.c_str(), accessdev_idnum_code, id.c_str());


        int len_origin=strlen((char*)temp_data_str);//数据段长度

        string data_len  = ensureLen_4(len_origin);
        /*进行循环冗余码的计算并保证为4位并完成HJ212的数据包封装*/
        unsigned int hjcrc_air=CRC16_Checkout(temp_data_str,len_origin);
        string hj212pacet=ensure_crc4_packet(hjcrc_air,data_len,temp_data_str);
        cout<<"HJ212 Airdata packet:"<<hj212pacet<<endl;

        string airData06 = packet06(hj212pacet, RS485_B);                  //封装为私有协议06包
        transMessage.push(airData06);
        cout<<"Obtaining data successfully"<<endl;

        /********************************************************************等待ISR下发采集命令************************************************************************/      
        S=pthread_mutex_lock(&mtx_sensor);
        if(S<0) perror("pthread_mutex_lock()");
        while(iS_getsensor==0){  //检查是否收到了03请求消息命令
            pthread_cond_wait(&cond_sensor,&mtx_sensor);  //如果没有在这里阻塞等待ISR给SAP下03命令
        }
        while(iS_getsensor>0){    
            iS_getsensor=0;
        }
        pthread_mutex_unlock(&mtx_sensor);   
    }
}




/**
*   获取新的大气传感器数据
*   @return  -1 获取失败
*   外加了温湿度485模块
*/
void* get_newsensor_data(void *arg)
{
    cout << "Thread: start get_newsensor_data" << endl;

    CSoftwareWdt *g_CsoftwareWdt=(CSoftwareWdt *)arg;
    const char*  threadname="get_sensor_data";
    wdtNewSensorId = g_CsoftwareWdt->RequestSoftwareWdtID(threadname,softdogTimeout + 10);//当前此线程注册
    uint16_t tab_reg[64] = { 0 };
    int S;
    string cpu_mem="";
    modbus_t *ctx = NULL;
    ctx = modbus_new_rtu(MODBUS_TTY, BAUD_RATE, 'N', 8, 1);
    modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
    //设置RTU（可选）
    modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_DOWN);
    modbus_set_debug(ctx, TRUE);

    /*定义的气体监测变量,与HJ212前缀编码*/
    float sensor_co=-999,sensor_o2=-999,sensor_ch4=-999,sensor_o3=-999,sensor_h2s=-999,sensor_so2=-999,sensor_nh3=-999,sensor_no2=-999,sensor_no=-999,sensor_co2=-999,sensor_humid=-999,sensor_temper=-999,sensor_pm1=-999,sensor_pm25=-999,sensor_pm10=-999;
    char sensor_so2_code[] = "a21026";
    char sensor_o2_code[] = "a19001";
    char sensor_ch4_code[] = "a05002";               //tgs813
    char sensor_no_code[] = "a21003";
    char sensor_nh3_code[] = "a21001";
    char sensor_no2_code[] = "a21004";
    char sensor_h2s_code[] = "a21028";
    char sensor_co2_code[] = "a05001";
    char sensor_co_code[] = "a21005";
    char sensor_temper_code[] = "a01001";
    char sensor_humid_code[] = "a01002";
    char sensor_pm25_code[] = "a34004";
    char sensor_pm10_code[] = "a34002";
    char sensor_pm1_code[] ="a34005" ;
    char sensor_o3_code[] ="a34003" ;//暂时后期修改
    /*定义的气体监测变量,与HJ212前缀编码*/
    //检查与从设备是否连接
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed:%s\n", modbus_strerror(errno));
        modbus_free(ctx);
        //return -1;
    }
    //设置应答延时（可选）
    while(1) //一直读取数据
    {
        //此线程进行软件喂狗操作
        g_CsoftwareWdt->KeepSoftwareWdtAlive(wdtNewSensorId);
        cout<<"thread of Airsensor feed dog success"<<endl;
        /*******************************************************读取大气电路板数据*************************************************/
        memset(tab_reg, 0, sizeof(tab_reg));
        modbus_set_slave(ctx, SLAVE_ADDR);   
        int regs = modbus_read_registers(ctx,0,26,tab_reg);
        if (regs == -1)
        {
            fprintf(stderr, "%s\n", modbus_strerror(errno));
            // return -1;
            printf("read registers error\n");

            sleep(4);
            continue;
        }
        sensor_co = tab_reg[0]*0.1;
        sensor_o2 = tab_reg[1]*0.1;
        sensor_ch4 = tab_reg[3]*0.1;
        sensor_o3 = tab_reg[4]*0.01;
        sensor_h2s = tab_reg[5]*0.01;
        sensor_so2 = tab_reg[6]*0.1;
        sensor_nh3 = tab_reg[7]*0.1;
        sensor_no2 = tab_reg[8]*0.1;
        sensor_no = tab_reg[9]*0.1;
        sensor_pm1 =tab_reg[12];
        sensor_pm10 = tab_reg[13];
        sensor_pm25 = tab_reg[14];
        sensor_humid = (tab_reg[24]>>8)& 0xFF;
        sensor_temper = tab_reg[24]& 0x00FF;
        sensor_co2 = tab_reg[25];
        sleep(1);
        /*******************************************************读取温湿度传感器数据*************************************************/
        memset(tab_reg, 0, sizeof(tab_reg));
        modbus_set_slave(ctx, 13);
        regs = modbus_read_registers(ctx,0,2,tab_reg);
        if (regs == -1)
        {
            fprintf(stderr, "%s\n", modbus_strerror(errno));
            // return -1;
            printf("read registers error\n");

            // sleep(4);
            // continue;
        }
        else
        {
        sensor_humid = tab_reg[0]*0.1;
        sensor_temper = tab_reg[1]*0.1;
        }

        /********************************************************************HJ212包的封装*************************************************************************/
        string hj_QN = "";  //时间
        //系统编码*5
        char hj_ST[] = "31";  
        //命令编码*7
        char hj_CN[] = "2011";  
        //访问密码*9
        char hj_PW[] = "123456";
        //设备唯一标识符*27
        string hj_MN = "";
        hj_MN = get_MN();
        //拆分包及应答标志*8
        char hj_Flag[] = "4";
        unsigned char temp_data_str[1024]={};
        //获取GPS信息
        string accessdev_gps = gps_lat + gps_lon;
        //string accessdev_gps = "NFFFFFFFF"; //测试用
        //获取时间戳消息
        hj_QN = hj_QN + time_now_to_string();
        //CPU和内存使用率
        cpu_mem=get_cpuOccupy();
        //打印信息
        printf("co:%f,02:%f,ch4:%f,o3:%f,h2s:%f,so2:%f,nh3:%f,no2:%f,No:%f,pm1:%f,pm10:%f,pm25:%f,humid:%f,temper:%f,co2:%f\n",\
        sensor_co,sensor_o2,sensor_ch4,sensor_o3,sensor_h2s,sensor_so2,sensor_nh3,sensor_no2,sensor_no,sensor_pm1,sensor_pm10,sensor_pm25,sensor_humid,sensor_temper,sensor_co2);
        /******************************hj212数据段的封装*********************/
        /*sprintf((char*)temp_data_str,\
        "QN=%s;ST=%s;CN=%s;PW=%s;MN=%s;Flag=%s;CP=&&Gps=%s;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf&&",\
        hj_QN.c_str(), hj_ST, hj_CN, hj_PW, hj_MN.c_str(),hj_Flag,accessdev_gps.c_str(),\ 
        sensor_so2_code,sensor_so2,sensor_o2_code,sensor_o2,sensor_ch4_code,sensor_ch4,sensor_no_code,sensor_no,sensor_nh3_code,sensor_nh3,\
        sensor_no2_code,sensor_no2,sensor_h2s_code,sensor_h2s,sensor_co2_code,sensor_co2,sensor_co_code,sensor_co,sensor_temper_code,sensor_temper,\
        sensor_humid_code,sensor_humid,sensor_o3_code,sensor_o3,sensor_pm25_code,sensor_pm25,sensor_pm10_code,sensor_pm10,sensor_pm1_code,sensor_pm1
        );*/
        sprintf((char*)temp_data_str,\
        "QN=%s;ST=%s;CN=%s;PW=%s;MN=%s;Flag=%s;CP=&&Gps=%s;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf&&",\
        hj_QN.c_str(), hj_ST, hj_CN, hj_PW, hj_MN.c_str(),hj_Flag,accessdev_gps.c_str(),\ 
        sensor_so2_code,sensor_so2,sensor_o2_code,sensor_o2,sensor_o3_code,sensor_o3,sensor_ch4_code,sensor_ch4,sensor_no_code,sensor_no,sensor_nh3_code,sensor_nh3,\
        sensor_no2_code,sensor_no2,sensor_h2s_code,sensor_h2s,sensor_co2_code,sensor_co2,sensor_co_code,sensor_co,sensor_temper_code,sensor_temper,\
        sensor_humid_code,sensor_humid,sensor_pm25_code,sensor_pm25,sensor_pm10_code,sensor_pm10
        );
        /******************************hj212数据段的封装***********************/
        //封装后的打印信息
        //printf("HJ212数据段：%s\n",temp_data_str);
        //sleep(4);
        /********************************************************************HJ212包的封装*************************************************************************/
        //获取数据段的长度，并确保为4个长度
        int len_origin=strlen((char*)temp_data_str);           
        string data_len=ensureLen_4(len_origin);
        /*进行循环冗余码的计算并保证为4位并完成HJ212的数据包封装*/
        unsigned int hjcrc_air=CRC16_Checkout(temp_data_str,len_origin);
        string hj212pacet=ensure_crc4_packet(hjcrc_air,data_len,temp_data_str); 
        cout<<"HJ212 Airdata packet:"<<hj212pacet<<endl;
        /********************************************************************HJ212包的封装*************************************************************************/

        /********************************************************************私有协议包的的封装*************************************************************************/      
        //string communicateType = to_string(CM->getSuccessId()); //当前通信模块类型
        string airData06 = packet06(hj212pacet, RS485_B);                  //封装为私有协议06包
        transMessage.push(airData06);
        cout<<"Obtaining data successfully"<<endl;
        /********************************************************************私有协议包的的封装*************************************************************************/ 

        /********************************************************************等待ISR下发采集命令************************************************************************/      
        S=pthread_mutex_lock(&mtx_sensor);
        if(S<0) perror("pthread_mutex_lock()");
        while(iS_getsensor==0){  //检查是否收到了03请求消息命令
            pthread_cond_wait(&cond_sensor,&mtx_sensor);  //如果没有在这里阻塞等待ISR给SAP下03命令
        }
        while(iS_getsensor>0){    
            iS_getsensor=0;
        }
        pthread_mutex_unlock(&mtx_sensor);
        sensor_co = -999;
        sensor_o2 = -999;
        sensor_ch4 = -999;
        sensor_o3 = -999;
        sensor_h2s = -999;
        sensor_so2 = -999;
        sensor_nh3 = -999;
        sensor_no2 = -999;
        sensor_no = -999;
        sensor_pm1 =-999;
        sensor_pm10 = -999;
        sensor_pm25 = -999;
        sensor_humid = -999;
        sensor_temper = -999;
        sensor_co2 =-999;     
    }
}

/**
*   通过集线器的方式获取所有大气传感器数据
*/
void* get_gassensor_data(void *arg)
{
    cout << "Thread: start get_gassensor_data" << endl;

    //看门狗注册
    CSoftwareWdt *g_CsoftwareWdt=(CSoftwareWdt *)arg;
    const char*  threadname="get_gassensor_data";
    wdtNewSensorId = g_CsoftwareWdt->RequestSoftwareWdtID(threadname,softdogTimeout + 10);//当前此线程注册
 
    int S;//信号量

    string cpu_mem="";
    

    int slave_address[11] = { 1,2,4,5,6,7,8,9,10,11,12};//定义从机地址
    uint16_t tab_reg[512] = { 0 };//接收缓存数组
    int regs = 0;//modbus返回值
    int index_flag = 0;//从机索引号

    //modbus_t初始化
    modbus_t *ctx1 = NULL;
    ctx1 = modbus_new_rtu(MODBUS_TTY, BAUD_RATE, 'N', 8, 1);
    //modbus_set_slave(ctx, SLAVE_ADDR);后面再设从机地址
    modbus_rtu_set_serial_mode(ctx1, MODBUS_RTU_RS485);

    //设置RTS（可选）
    modbus_rtu_set_rts(ctx1, MODBUS_RTU_RTS_DOWN);
    modbus_set_debug(ctx1, TRUE);

    /*定义的气体监测变量,与HJ212前缀编码*/
    float sensor_co,sensor_o2,sensor_ch4,sensor_o3,sensor_h2s,sensor_so2,sensor_nh3,sensor_no2,sensor_no,sensor_co2,sensor_humid,sensor_temper,sensor_pm1,sensor_pm25,sensor_pm10;

    char sensor_so2_code[] = "a21026";
    char sensor_o2_code[] = "a19001";
    char sensor_ch4_code[] = "a05002";               //tgs813
    char sensor_no_code[] = "a21003";
    char sensor_nh3_code[] = "a21001";
    char sensor_no2_code[] = "a21004";
    char sensor_h2s_code[] = "a21028";
    char sensor_co2_code[] = "a05001";
    char sensor_co_code[] = "a21005";
    char sensor_temper_code[] = "a01001";
    char sensor_humid_code[] = "a01002";
    char sensor_pm25_code[] = "a34004";
    char sensor_pm10_code[] = "a34002";
    char sensor_pm1_code[] ="a34005" ;
    char sensor_o3_code[] ="a34003" ;//暂时后期修改
    /*定义的气体监测变量,与HJ212前缀编码*/
    
    if (modbus_connect(ctx1) == -1)
    {
        fprintf(stderr, "Connection failed:%s\n", modbus_strerror(errno));
        modbus_free(ctx1);
        //return -1;
    }
    //设置应答延时（可选）
    while(1) //一直读取数据
    {
        //此线程进行软件喂狗操作
        g_CsoftwareWdt->KeepSoftwareWdtAlive(wdtNewSensorId);
        cout<<"thread of Airsensor feed dog success"<<endl;
        
        modbus_set_slave(ctx1, slave_address[index_flag]); //设置从机地址
        memset(tab_reg, 0, sizeof(tab_reg));//清空接收缓存

        switch (index_flag) {
        //1号从机CO
        case 0: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg); 
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_co=-999;
                sleep(2);
                break;
            }
            sensor_co=tab_reg[0]*0.1;
            sleep(2);
            break;

        //2号从机O2
        case 1: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg); 
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_o2=-999;
                sleep(2);
                break;
            }
            sensor_o2=tab_reg[0]*0.1;
            sleep(2);
            break;

        //3号从机CH4
        case 2: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_ch4=-999;
                sleep(2);
                break;
            }
            sensor_ch4=tab_reg[0]*0.1;
            sleep(2);
            break;

        //4号从机O3
        case 3: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_o3=-999;
                sleep(2);
                break;
            }
            sensor_o3=tab_reg[0]*0.01;
            sleep(2);
            break;

        //5号从机H2S
        case 4: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_h2s=-999;
                sleep(2);
                break;
            }
            sensor_h2s=tab_reg[0]*0.01;
            sleep(2);
            break;

        //6号从机SO2
        case 5: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_so2=-999;
                sleep(2);
                break;
            }
            sensor_so2=tab_reg[0]*0.01;
            sleep(2);
            break;

        //7号从机NH3
        case 6: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_nh3=-999;
                sleep(2);
                break;
            }
            sensor_nh3=tab_reg[0]*0.1;
            sleep(2);
            break;

        //8号从机NO2
        case 7: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_no2=-999;
                sleep(2);
                break;
            }
            sensor_no2=tab_reg[0]*0.1;
            sleep(2);
            break;

        //9号从机NO
        case 8: 
            regs = modbus_read_registers(ctx1, 6, 1, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_no=-999;
                sleep(2);
                break;
            }
            sensor_no=tab_reg[0]*0.1;
            sleep(2);
            break;

        //10号从机PM温湿度
        case 9: 
            regs = modbus_read_registers(ctx1, 0, 5, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                sensor_pm10=-999;
                sensor_pm25=-999;
                sensor_pm1=-999;
                sensor_humid=-999;
                sensor_temper=-999;
                sleep(2);
                break;
            }
            sensor_humid=tab_reg[0]*0.1;
            sensor_temper=tab_reg[1]*0.1;
            sensor_pm25=tab_reg[2];
            sensor_pm10=tab_reg[3];
            sensor_pm1=tab_reg[4];
            sleep(2);
            break;
/***************************************************************************************/
        //11号从机温湿度co2
        case 10: 
            regs = modbus_read_registers(ctx1, 0, 3, tab_reg);  
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                // sensor_humid=-999;
                // sensor_temper=-999;
                sensor_co2=-999;
                sleep(2);
                break;
            }
            // sensor_humid=tab_reg[0]*0.1;
            // sensor_temper=tab_reg[1]*0.1;
            sensor_co2=tab_reg[2];
            sleep(2);
            break;
        }
/***************************************************************************************/
        // //12号从机CO2
        // case 11: 
        //     regs = modbus_read_registers(ctx1, 2, 1, tab_reg);  
        //     if (regs == -1) {
        //         fprintf(stderr, "%s\n", modbus_strerror(errno));
        //         //return false;
        //         sensor_co2=-999;
        //         sleep(2);
        //         break;
        //     }
        //     sensor_co2=tab_reg[0];
        //     sleep(2);
        //     break;
        //}
       
        // int regs = modbus_read_registers(ctx,0,26,tab_reg);
        // if (regs == -1)
        // {
        //     fprintf(stderr, "%s\n", modbus_strerror(errno));
        //     // return -1;
        //     printf("read registers error\n");

        //     sleep(4);
        //     continue;
        // }
        // sensor_co = tab_reg[0]*0.1;
        // sensor_o2 = tab_reg[1]*0.1;
        // sensor_ch4 = tab_reg[3]*0.1;
        // sensor_o3 = tab_reg[4]*0.01;
        // sensor_h2s = tab_reg[5]*0.1;
        // sensor_so2 = tab_reg[6]*0.1;
        // sensor_nh3 = tab_reg[7]*0.1;
        // sensor_no2 = tab_reg[8]*0.1;
        // sensor_no = tab_reg[9]*0.1;
        // sensor_pm1 =tab_reg[12];
        // sensor_pm10 = tab_reg[13];
        // sensor_pm25 = tab_reg[14];
        // sensor_humid = (tab_reg[24]>>8)& 0xFF;
        // sensor_temper = tab_reg[24]& 0x00FF;
        // sensor_co2 = tab_reg[25];
        
        if (index_flag == 11) 
        {
        index_flag = -1;
/********************************************************************HJ212包的封装*************************************************************************/
         string hj_QN = "";  //时间
        //系统编码*5
        char hj_ST[] = "31";  
        //命令编码*7
        char hj_CN[] = "2011";  
        //访问密码*9
        char hj_PW[] = "123456";
        //设备唯一标识符*27
        string hj_MN = "";
        hj_MN = get_MN();
        //拆分包及应答标志*8
        char hj_Flag[] = "4";
        unsigned char temp_data_str[1024]={};
        //获取GPS信息
        string accessdev_gps = gps_lat + gps_lon;
        //string accessdev_gps = "NFFFFFFFF"; //测试用
        //获取时间戳消息
        hj_QN = hj_QN + time_now_to_string();
        //CPU和内存使用率
        cpu_mem=get_cpuOccupy();
        //打印信息
        printf("co:%f,02:%f,ch4:%f,o3:%f,h2s:%f,so2:%f,nh3:%f,no2:%f,No:%f,pm1:%f,pm10:%f,pm25:%f,humid:%f,temper:%f,co2:%f\n",\
        sensor_co,sensor_o2,sensor_ch4,sensor_o3,sensor_h2s,sensor_so2,sensor_nh3,sensor_no2,sensor_no,sensor_pm1,sensor_pm10,sensor_pm25,sensor_humid,sensor_temper,sensor_co2);
        
        
        /******************************hj212数据段的封装*********************/
        
        sprintf((char*)temp_data_str,\
        "QN=%s;ST=%s;CN=%s;PW=%s;MN=%s;Flag=%s;CP=&&Gps=%s;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf;%s-Rtd=%08.2lf&&",\
        hj_QN.c_str(), hj_ST, hj_CN, hj_PW, hj_MN.c_str(),hj_Flag,accessdev_gps.c_str(),\ 
        sensor_so2_code,sensor_so2,sensor_o2_code,sensor_o2,sensor_o3_code,sensor_o3,sensor_ch4_code,sensor_ch4,sensor_no_code,sensor_no,sensor_nh3_code,sensor_nh3,\
        sensor_no2_code,sensor_no2,sensor_h2s_code,sensor_h2s,sensor_co2_code,sensor_co2,sensor_co_code,sensor_co,sensor_temper_code,sensor_temper,\
        sensor_humid_code,sensor_humid,sensor_pm25_code,sensor_pm25,sensor_pm10_code,sensor_pm10
        );
        
        /******************************hj212数据段的封装***********************/
        //封装后的打印信息
        //printf("HJ212数据段：%s\n",temp_data_str);
        //sleep(4);
        /********************************************************************HJ212包的封装*************************************************************************/
        //获取数据段的长度，并确保为4个长度
        int len_origin=strlen((char*)temp_data_str);           
        string data_len=ensureLen_4(len_origin);
        /*进行循环冗余码的计算并保证为4位并完成HJ212的数据包封装*/
        unsigned int hjcrc_air=CRC16_Checkout(temp_data_str,len_origin);
        string hj212pacet=ensure_crc4_packet(hjcrc_air,data_len,temp_data_str);
        cout<<"HJ212 Airdata packet:"<<hj212pacet<<endl;
        /********************************************************************HJ212包的封装*************************************************************************/

        /********************************************************************私有协议包的的封装*************************************************************************/      
        //string communicateType = to_string(CM->getSuccessId()); //当前通信模块类型
        string airData06 = packet06(hj212pacet, RS485_B);                  //封装为私有协议06包
        transMessage.push(airData06);
        cout<<"Obtaining data successfully"<<endl;
        /********************************************************************私有协议包的的封装*************************************************************************/ 

        /********************************************************************等待ISR下发采集命令************************************************************************/      
        S=pthread_mutex_lock(&mtx_sensor);
        if(S<0) perror("pthread_mutex_lock()");
        while(iS_getsensor==0){  //检查是否收到了03请求消息命令
            pthread_cond_wait(&cond_sensor,&mtx_sensor);  //如果没有在这里阻塞等待ISR给SAP下03命令
        }
        while(iS_getsensor>0){    
            iS_getsensor=0;
        }
        pthread_mutex_unlock(&mtx_sensor); 
        }
        index_flag++;     
    }
    
}



/**
*   发送心跳包，表明设备是否在线
*   @return
*/
void heartbeat()
{
    while (1)
    {
        string t_heart = "";
        t_heart = t_heart + "$" + "EF" + "1" + id + net_id + "12" + mac + id + "@";
        mutex_send_message.lock();
        send_message.push(t_heart);
        mutex_send_message.unlock();
        sleep(150);
    }
}
void* send_mess(void* arg)
{
    cout<<" start trans message to getway"<<endl;
    uint8_t check_buf[RX_SIZE];    //接收字符串
    uint8_t RX_buf[RX_SIZE];    //接收字符串
    int ret = -1;
    fd_set fdset;
    FD_ZERO(&fdset);
    struct timeval tv;  
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    CSoftwareWdt *g_CsoftwareWdt=(CSoftwareWdt *)arg;
    int count = 0;
    const char* const threadname="trans_message";
    int wdt_id = -1;
    wdt_id = g_CsoftwareWdt->RequestSoftwareWdtID(threadname,softdogTimeout);//线程注册
    int flag = true; //标志位
    while(1)
    {
        int deviceFd=CM->getSuccessFd();
        cout<<"Enable FD Num："<<deviceFd<<endl;
        FD_SET(deviceFd, &fdset);
        ret = select(deviceFd + 1, &fdset, NULL, NULL, NULL);
        if(ret>0)
        {
           if (FD_ISSET(deviceFd, &fdset))
           {
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read(deviceFd, RX_buf, RX_SIZE);
                if(ret == 0) {          //代表服务器主动断开连接，此时重新注册  /**新加的功能**/
                    cout<<"<<<<<<<<<<<<<ISR Quit<<<<<<<<<<<<<<"<<endl;
                    cout<<"<<<<<<<<<<<<<SAP Will reConnect<<<<<<<<<<<<<<"<<endl;
                    //g_CsoftwareWdt->ReleaseSoftwareWdtID(threadname, wdt_id);// 解除对此线程的监控
                    return 0;           //直接退出此线程，等待重新注册
                }
                if (ret <= 58)
                {
                    string recv_message(RX_buf, RX_buf + ret);
                    cout<<"Message From The Gateway："<<recv_message<<endl;
                    string recv_protocal = recv_message.substr(1, 2);
                    if (recv_protocal == REQ_DATA)   // 收到03包
                    {
                        string recv_mac = recv_message.substr(22, 16);   //获取下发的mac地址，设备唯一码
                        if (recv_mac == mac)  //判断是否是注册成功的ISR发送的03包
                        {
                            count ++;
                            cout << "当前消息报计数器:" << count <<endl; 
                            g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
                            cout<<"thread of trans_message feed dog success"<<endl;
                            string communicate_method = recv_message.substr(38, 4); //获取ISR下发的传输位

                            pthread_mutex_lock(&mtx_sensor);    //加锁，访问全局共享资源
                            iS_getsensor=1;
                            iS_getshipsensor=1;
                            pthread_mutex_unlock(&mtx_sensor);  //解锁，释放全局共享资源
                            //pthread_cond_signal(&cond_sensor);//发出信号，唤醒读传感器数据的线程,唤醒单个线程
                            pthread_cond_broadcast(&cond_sensor);
                            while(!readMessage.empty()){ //检查是否读取到未发送的数据包，如果有就先发送这些数据包
                                flag = false;
                                cout<<"当前未发送的缓存队列还剩余： "<< readMessage.size()<<endl;
                                string str = readMessage.front(); //从缓冲队列取出一条数据进行发送
                                readMessage.pop();
                                ret = write(deviceFd, str.c_str(), strlen(str.c_str()));
                                if(ret==0)
                                {
                                    //continue;
                                }
                                if (ret < 0)
                                {
                                    cout << "TransFd = " << deviceFd << endl;
                                    cout << "data_packet = " << str << endl;
                                    cout << "ret = " << ret << endl;
                                    cout << "Trans error(deviceFd): " << strerror(errno) << endl;
                                    //continue;
                                }else
                                {
                                    sleep(2);
                                    g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
                                    cout<<"发送离线缓存包"<<endl;
                                    cout << "hj_crc: " << hj_crc << endl;
                                    cout << "=======================" << endl;
                                    cout << "data_packet = " << str << endl;
                                    cout << "Trans success(deviceFd) " <<deviceFd<<endl;
                                    cout << "包的长度" <<strlen(str.c_str())<< endl;
                                    //continue;
                                }
                                //重传的时候要对其他监控的线程进行喂狗操作，否则会挂机
                                g_CsoftwareWdt->KeepSoftwareWdtAlive(wdtNewSensorId);
                                g_CsoftwareWdt->KeepSoftwareWdtAlive(wdtShipId);
                            }
                            if(flag == false){
                                flag = true;  //当前已经没有可发送的数据包了
                                continue;
                            }
                            if(!transMessage.empty()){
                                cout<<"当前缓存队列剩余： "<< transMessage.size()<<endl;
                                string str = transMessage.front(); //从缓冲丢列取出一条数据进行发送
                                transMessage.pop();
                                ret = write(deviceFd, str.c_str(), strlen(str.c_str()));
                                if(ret==0)
                                {
                                    continue;
                                }
                                if (ret < 0)
                                {
                                    cout << "TransFd = " << deviceFd << endl;
                                    cout << "data_packet = " << str << endl;
                                    cout << "ret = " << ret << endl;
                                    cout << "Trans error(deviceFd): " << strerror(errno) << endl;
                                    continue;
                                }else
                                {
                                    cout << "hj_crc: " << hj_crc << endl;
                                    cout << "=======================" << endl;
                                    cout << "data_packet = " << str << endl;
                                    cout << "Trans success(deviceFd) " <<deviceFd<<endl;
                                    cout << "包的长度" <<strlen(str.c_str())<< endl;
                                    continue;
                                }
                            }
                        }
                    }else if(recv_protocal == TIME_SEND){ // 收到20包 
                        count = 0;
                        g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
                        packet20(recv_message); // 更新时间戳消息
                    }      
                }
            } 
        }
    }
}

int main()
{ 

    /*int pid, i;
    pid = fork();
    if(pid > 0) {
        exit(0); //结束父进程，使得子进程成为后台进程
    }
    else if(pid < 0) { 
        return -1;
    }
 
    //建立一个新的进程组,在这个新的进程组中,子进程成为这个进程组的首进程,以使该进程脱离所有终端
    setsid();
 
    //再次新建一个子进程，退出父进程，保证该进程不是进程组长，同时让该进程无法再打开一个新的终端
    pid=fork();
    if( pid > 0) {
        exit(0);
    }
    else if( pid< 0) {
        return -1;
    }
 
    //关闭所有从父进程继承的不再需要的文件描述符
    for(i=0;i< 1024;close(i++));
 
    //改变工作目录，使得进程不与任何文件系统联系
    chdir("/home/root");
 
    //将文件创建屏蔽字设置为0
    umask(0);*/

    /*打开用于监控的文件，并将此进程的进程号，以覆盖的方式写入*/
    pid_t pt = getpid(); //返回当前进程ID
    const char* ptbuffer = to_string(pt).c_str();
    int fdWatch = open(watchPathName, O_WRONLY | O_TRUNC);  //O_TRUNC 清空文件内容
    if(fdWatch < 0) perror("main open:");
    if(  fdWatch > 0){
      int ret =   write(fdWatch, ptbuffer, sizeof(ptbuffer)); 
      if(ret < 0){
        perror("错误：");
      }else{
        close(fdWatch);
      }
    }


    

    system("./power_12v.sh on");
    sleep(1);
    /*初始化一些必要的结构体*/
    CSoftwareWdt *g_CsoftwareWdt = new CSoftwareWdt();
    struct monitor485 *monitor=new monitor485(&transMessage, g_CsoftwareWdt);   //用于无人船时给注释掉，国家项目加上                                
    /*初始化一些必要的结构体*/

    //pthread_t tid_getSensor, tid_getLan, tid_transMessage, tid_gps, tid_ship_data, tid_softwd, tid_monitor485, tid_deviceRegist;
    pthread_create(&tid_gps,NULL,GET_GPS,NULL);    //开启GPS获取线程
  
    sleep(1);

    int ret;                    //错误参数保存
    int fd_max;                 //最大文件描述符
    fd_set fdset;               //文件描述符集合
    //signal(SIGINT, stop);       //注册退出函数
    uint8_t RX_buf[RX_SIZE];    //接收字符串

    
   

    //get_newsensor_data(NULL);//*******************************************************************************************测试用

    ret = dev_init();           //设备初始化，数组在进行调用传值的时候可以直接传递数组名
    if (-1 == ret) {
        printf("Init communicate device error start reboot!\n");
        stop(1);
        ptbuffer = to_string(0).c_str();
        fdWatch = open(watchPathName, O_WRONLY);
        if(  fdWatch > 0){
            int ret =  write(fdWatch, ptbuffer, sizeof(ptbuffer)); 
            if(ret < 0){
                perror("错误：");
            }else{
                close(fdWatch);
            }
        }
        cout<<"进程退出"<<endl;
        delete g_CsoftwareWdt;
        return 0;
    }
    else
    {
        cout << "Init communicate device success" << endl;
        pthread_create(&tid_monitor485,NULL,RS485_Monitor,monitor);   //开启串口监听线程   用于无人船时给注释掉,国家项目加上
        //获取南水所的数据
        sleep(1);
        pthread_create(&tid_getLan,NULL,Get_L_data,NULL);
        //设备初始化成功后，首先进行注册操作
        get_mac();              //首先获取接入设备的mac地址
        cpu_occupy=get_cpuOccupy();
        cout<<"CPU Usage Rate："<<cpu_occupy<<endl;
        vector<vector<int>> vec;
        vec=CM->getALLIfd();
        struct deviceRegist *Regist = new deviceRegist(g_CsoftwareWdt, &vec); //为注册函数准备参数
        pthread_create(&tid_deviceRegist, NULL, device_regist, Regist);
        /*开启软件看门狗线程----->监测关键的线程状态是否正常*/
        pthread_create(&tid_softwd,NULL,softwarewd,g_CsoftwareWdt);
        void* ret;
        if(pthread_join(tid_deviceRegist,&ret) == 0)          //等待注册线程正常退出,如果没有注册成功的退出，执行进程的退出
        {
            if( (int)ret == -1){
                cout<<"未注册成功"<<endl;
                sleep(30);
                ptbuffer = to_string(0).c_str();
                fdWatch = open(watchPathName, O_WRONLY);
                if(  fdWatch > 0){
                    int ret =  write(fdWatch, ptbuffer, sizeof(ptbuffer)); 
                    if(ret < 0){
                        perror("错误：");
                    }else{
                        close(fdWatch);
                    }
                } 
                return 0;  
            } 
        }
        cout<<"<<<<<<<<<<<<<<<<<<<<<Device Registration Successful<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
        int numRead = readFromFile(readMessage); //将存储于于本地文档内的数据读进缓存队列
        cout << "读取到未发送的数据包数：" << numRead;
        //device_regist(vec);
        //device_regist_lan();                        //设备注册完成后，开启gps，数据采集，数据处理线程
        //pthread_create(&tid_getSensor,NULL,get_sensor_data,g_CsoftwareWdt);
        pthread_create(&tid_getSensor,NULL,get_newsensor_data,g_CsoftwareWdt);
        //获取无人船上面的传感器数据
        sleep(1);
        pthread_create(&tid_ship_data,NULL,get_ship_data,g_CsoftwareWdt); //用于无人项目加上，国家项目注释掉
        //开启心跳包线程
        // thread th_heartbeat(heartbeat);
        // th_heartbeat.detach();
        //开启发送线程
        pthread_create(&tid_transMessage,NULL,send_mess,g_CsoftwareWdt);

        //做一个对线程回收的机制
    }
    cout<<"看门狗线程释放之前"<<endl;
    pthread_join(tid_softwd, NULL);
    cout<<"软件看门狗线程释放"<<endl;
    int writeNum = dataTofile(transMessage);
    cout<<"我的测试"<<endl;
    cout << "写入的数据包数：" << writeNum;
    ptbuffer = to_string(0).c_str();
    //fdWatch = open(watchPathName, O_WRONLY | O_TRUNC);
    fdWatch = open(watchPathName, O_WRONLY);
    if(  fdWatch > 0){
        int ret =  write(fdWatch, ptbuffer, sizeof(ptbuffer)); 
        if(ret < 0){
            perror("错误：");
        }else{
            close(fdWatch);
        }
    }
    //mytest("进程退出\n");
    cout<<"进程退出"<<endl;
    return 0;
}


