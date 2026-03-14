#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <vector>
#include <iomanip>
#include <mutex>
#include <queue>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>
#include "driver/lora.h"  
#include "driver/bluetooth.h"
#include "driver/usbctl.h"
#include "driver/protocol_process.h"
#include "/home/wp/guoxiang/apDevice/libmodbus/include/modbus/modbus.h"

#include "driver/get.h"
#include<pthread.h>  

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
#define ZD_LAN_SERVER_IP        "192.168.31.101"          //中电莱斯传数据本地服务器ip
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
#define REQ_ACC_HEART           "EF"                    //接入设备心跳包
#define REQ_GATE_HEART          "FF"                    //网关设备心跳包

using namespace std;
/*******************************************************
            调试版本
(1)9月13日，修改03协议字段
*******************************************************/

int fd_lora = -1, fd_wifi = -1, fd_bt = -1, fd_lan = -1,fdL_lan=-1;//通信设备描述符
vector<int>device_id;                                   //设备描述符容器
int fd_num;                                             //文件描述符数量
string id = "FF", net_id = "0000", mac, s_req_fre, bt_mac = "",Isr_mac="";      //设备id，设备网络id，设备mac，问询频率,Isr_mac地址
int monitor_time = 4;
string current_time = "";
char communicate_status[] = "0000";                     //通信方式描述数组
int frequency = 0;
unsigned int hj_crc; 
/**************************************定义的一些全局共享变量*********************************/
int iS_getsensor=0;
/************************************为线程同步定义的锁和条件变量*****************************/
static pthread_mutex_t mtx_sensor=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_sensor=PTHREAD_COND_INITIALIZER;
/*******************************************************************************************/                                   

string hj_data_packet = "";                             //hj212数据包
char send_data_packet[1500];                            //大气私有协议数据包
char hj_njdata_packet[1500];                            //水土气私有协议数据包
char hj_njdata_rece[1500];                              //南京传过来的数据包
queue<string>send_message;                              //消息队列：发送消息
queue<string>recv_message;                              //消息队列：接收消息
queue<string> recvnj_message;                       //消息队列：存放接收南水所的消息
string len_j212="";                                      //记录总的HJ212数据包长度
string cpu_occupy="";

 string gps_lat= "NFFFF";
 string gps_lon="EFFFFF";                               //东经 //存储GPS经纬度
int   len_private1,len_private2;                                      //总的大气私有封包长度、水土气私有包长度
int flag_wifi=0;                                          //WIFI连接到ISR成功的标志
int flag_nj=0;                                              //接收到南京数据的标志位
mutex mutex_gps;                                        //互斥锁，锁住gps
mutex mutex_send_message;                               //互斥锁，锁住send_message
mutex mutex_recvnj_message;                             //互斥锁，锁住消息队列

struct sockaddr_in wifi_ser_addr;                       //wifi的TCP
struct sockaddr_in lan_ser_addr;                        //lan的TCP绑定地址（ip和端口号）
struct sockaddr_in Llan_ser_addr;                       //接收南京的本地服务器
struct sockaddr_in Llan_cli_addr;                                             /**
                                                        *   中断函数，用户停止程序使用
                                                        *   @param signum 捕获的信号量
                                                        *   @return
                                                        *
                                                        */
void stop(int signum)
{
#ifdef DEBUG 
    printf("Stop the program!\n");
#endif 
    string close_DISCONNECT = "AT+DISCON=1\r\n";
    close(fd_wifi);
    close(fd_lora);
    close(fd_bt);
    close(fd_lan);
    close(fdL_lan);
    system("/home/root/jd_open.sh");
    sleep(1);
    write_port(fd_bt, close_DISCONNECT.c_str(), strlen(close_DISCONNECT.c_str()));
    system("/home/root/jd_close.sh");
    sleep(2);
    system("echo 0 > /sys/class/leds/red/brightness");
    system("echo 0 > /sys/class/leds/yellow/brightness");
    system("echo 0 > /sys/class/leds/green/brightness");
    system("/home/root/power_12v.sh off");
    system("hwclock -w");  

    system("killall wpa_supplicant");            //关闭所有wpa_supplicant进程
    sleep(1);
    system("killall udhcpc");                    //关闭所有udhcpc进程
    exit(0);
}

string get_cpuOccupy(){                           //获取CPU使用率
    MEM_OCCUPY mem_stat;  
    CPU_OCCUPY cpu_stat1;  
    CPU_OCCUPY cpu_stat2;
    int mem_use;
    string mem;
    int cpu_use;
    string cpu;
    string mem_cpu;
    string cpu_mem;
    //获取内存
    //(MemTotal - MemFree)/ MemTotal
    get_memoccupy((MEM_OCCUPY *)&mem_stat);  
    mem = cal_memoccupy((MEM_OCCUPY *)&mem_stat);

   // cout << mem << endl;
    //第一次获取cpu使用情况  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);  
    usleep(100000);  
    //第二次获取cpu使用情况  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);  
    //计算cpu使用率  
    cpu =cal_cpuoccupy((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);
    //mem_cpu=mem+cpu;
    cpu_mem=cpu+mem;
    return cpu_mem;
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
    socklen_t   len;
    fd_set  rset, wset;     
    struct timeval  tval;

    // 设置 socket 为非阻塞
    if ((flags = fcntl(sockfd, F_GETFL, 0)) == -1) {   //获取文件状态
        perror("fcntl F_GETFL");
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {   //
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
        device_id.push_back(fd_bt);
#ifdef DEBUG 
        printf("Init success：bluetooth. Device descriptor = %d\n", fd_bt);
#endif
        communicate_status[3] = '1';
        system("echo 1 > /sys/class/leds/green/brightness");
    }
    sleep(2);

    //-----------------------------------------------

    //初始化LAN南水所有线
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
        Llan_ser_addr.sin_port = htons(LAN_SERVER_PORT);
        bind(fdL_lan,(struct sockaddr *)&Llan_ser_addr,sizeof(Llan_ser_addr));
        listen(fdL_lan,5);
#ifdef DEBUG
        printf("Init success: LanLSS. Device descriptor = %d\n", fdL_lan);
#endif
    }
    //-----------------------------------------------
    //初始化LAN有线的方式
    //首先设备wlan0的ip为192.168.2.2
    system("ifconfig eth1 192.168.2.2 netmask 255.255.255.0");
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
        printf("lan: connect server success\n");
        printf("Init success: Lan. Device descriptor = %d\n", fd_lan);
#endif
        int lan_fd_connect = connect_nonb(fd_lan,(struct sockaddr*)&lan_ser_addr,sizeof(lan_ser_addr), timeout);    //连接服务器
        if(lan_fd_connect < 0)
        {
#ifdef DEBUG 
            perror("lan: connect");
#endif 
        }
        else
        {
#ifdef DEBUG
            printf("lan: connect server success\n");
            printf("Init success: Lan. Device descriptor = %d\n", fd_lan);
#endif
            
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
    FILE *fp;
    fp = freopen("./mac.txt", "r", stdin);
    if (fp == NULL)
    {
        printf("get mac failed\n");
    }
    else
    {
        getline(cin, mac);
    }
    fclose(stdin);
    fclose(fp);
    return;
}



/**
*   获取采集设备唯一标识MN
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
void device_regist()
{
#ifdef DEBUG
    cout << "device_regist" << endl;
#endif
    struct timeval tv1;
    struct timeval tv2;
    int ret;
    fd_set fdset;               //文件描述符集合
    FD_ZERO(&fdset);
    FD_SET(fd_lora, &fdset);
    uint8_t RX_buf[RX_SIZE];    //接收字符串
    string regist_message = "";  // string accessdev_gps = gps_lat + "," + gps_lon;
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
                if(17==protoc_17(RX_buf)){
                    return;
                }else{
                    
                }
             }
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
    token = strstr(line, "V");   //去掉不需要的数据
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
    fd_set rest,allset; 
    int client[256];//是为了保存已经建立连接并返回的新套接字描述符
    int i,maxi,maxfd,confd,sockfd;
    int client_num=0;
    //char buf[1024];
    FD_ZERO(&allset);
    FD_SET(fdL_lan,&allset);
    socklen_t cliaddr_len;
    ssize_t n;
    for(i=0;i<64;i++){
        client[i]=-1;
    }
    maxfd=fdL_lan;
    maxi=-1;
    char temp[1500]={0};
    cout<<"--------等待南水所连接--------"<<endl;

    while(1){
        rest=allset;
        int ret=select(maxfd+1,&rest,NULL,NULL,NULL);
        if(ret<0){
            printf("select error");
            //break;
        } //有客户端连接
        if(FD_ISSET(fdL_lan,&rest)){
             cliaddr_len=sizeof(Llan_cli_addr);
             confd=accept(fdL_lan,(struct sockaddr*)&Llan_cli_addr,&cliaddr_len);
             client_num++;//连接的客户端数加1
             cout<<"--------连接成功--------"<<endl;
             cout<<"-------文件描述符："<<confd<<"--------"<<endl;
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
                if(( n=read(sockfd,hj_njdata_rece,1500))==0){
                    client_num--;
                    cout<<"《《《《《《《《《《《《         断开连接     》》》》》》》》》》》》"<<"等待重连"<<endl;
                     cout<<"《《《《《《《《《《《《  当前的客户端连接数  》》》》》》》》》》》》："<<client_num<<endl; 
                    close(sockfd);           // 当client关闭链接时，服务器端也关闭对应链接 
                    FD_CLR(sockfd, &allset); // 解除select监控此文件描述符 
                    memset(hj_njdata_rece,0,sizeof(hj_njdata_rece));
                    client[i] = -1;
                    //break;

                }else{
#ifdef DEBUG
                  cout<<"《《《《《《《《《《《《    接收到南水所的数据  》》》》》》》》》》》》："<<hj_njdata_rece<<endl;
#endif              
                   cout<<"《《《《《《《《《《《《 当前的客户端连接数  》》》》》》》》》》》》："<<client_num<<endl; 
                    strcpy(temp,hj_njdata_rece); 
                    recvnj_message.push(temp);   
                   // flag_nj=1;
                    write(sockfd,hj_njdata_rece,n);
                    //while(flag_nj==1){
                     //   if (flag_nj==0)break; //如果此次接收到的数据未发送出去，就一直停在这里等待发送
                    //}
                    memset(hj_njdata_rece,0,sizeof(hj_njdata_rece));

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
    fd = open((const char*)dev, O_RDWR | O_NOCTTY);
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




/**
*   将int型转换成十六进制字符串
*   @param num 需要转换的数据
*   @return
*
*/
string switch10_16(int num)  //规定余数输出格式的函数，0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
{
    switch(num){
        case 0:return"0";
                    break;
        case 1:return"1";
                    break;
        case 2:return"2";
                    break;
        case 3:return"3";
                   break;
        case 4:return"4";
                   break;
        case 5:return"5";
                   break;
        case 6:return"6";
                   break;
        case 7:return"7";
                    break;
        case 8:return"8";
                    break;
        case 9:return"9";
                    break;
        case 10:return"A";
                      break;
        case 11:return"B";
                      break;
        case 12:return"C";
                      break;
        case 13:return"D";
                      break;
        case 14:return"E";
                      break;
        case 15:return"F";
                      break;
    }
}

 int change(long long int num)  //转换函数，把十进制转换成十六进制
{
    if(num<16)    //如果要转换的数小于16，直接找到格式函数，把转换后的数加到字符串后面
    {
        len_j212+=switch10_16(num);
        return 0;
    }
    if(num>16) //如果要转换的数大于16
    {
        change(num/16);   //一直除16，相当于辗转相除法，不懂可以参考十进制算十六进制的过程
        len_j212+=switch10_16(num%16); //把余数加到字符串里
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


/**
*   获取传感器数据
*   @return  -1 获取失败
*
*/
void* get_sensor_data(void *arg)
{
#ifdef DEBUG
    cout << "Thread: start get_sensor_data" << endl;
#endif
    sleep(5);
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
    uint8_t temp_hj_str[1024] = {};
    int sensor_so2work, sensor_so2au, sensor_ke25, sensor_tgs822, sensor_tgs813, sensor_tgs2602, sensor_tgs2620;
    int sensor_mq131, sensor_nob4work, sensor_nob4au, sensor_nh3b1work, sensor_nh3b1au, sensor_no2b43fwork, sensor_no2b43fau, sensor_h2sb4work, sensor_h2sb4au;
    double sensor_t6703, sensor_jxmco, sensor_temper, sensor_humid, sensor_pm25, sensor_pm10;
    while (1)  //一直读取数据
    {
        memset(temp_data_str, 0, sizeof(temp_data_str));
        memset(temp_hj_str, 0, sizeof(temp_hj_str));
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
        accessdev_gps = "N2932E10636";

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


        int out_data_len;  //数据段长度

        out_data_len = strlen((char*)temp_data_str);

        string data_len_str = "";
        hj_crc = CRC16_Checkout(temp_data_str, out_data_len);





        char temp_data_len[10] = { 0 }; 
        sprintf((char*)temp_data_len, "%d", out_data_len);
        if (out_data_len < 10)
        {

            data_len_str = data_len_str + "000" + temp_data_len;

        }
        else if (out_data_len < 100)
        {
            data_len_str = data_len_str + "00" + temp_data_len;

        }
        else if (out_data_len < 1000)
        {
            data_len_str = data_len_str + "0" + temp_data_len;
        }
        else if (out_data_len < 10000)
        {
            data_len_str = data_len_str + temp_data_len;
        }
        else
        {
            cout << "data have outflow" << endl;
        }

        //data_len_hex =  data_len_hex + i2hex(out_data_len);



        hj_data_packet = "";

        //********************************保证hj212校验位4位封装整个HJ212************************************************
        if(hj_crc > 4095)  //FFF
        {
            sprintf((char*) temp_hj_str,"##%s%s%x", data_len_str.c_str(), temp_data_str, hj_crc);
        }
        else if(hj_crc > 255)  //FF
        {
            sprintf((char*)temp_hj_str, "##%s%s0%x", data_len_str.c_str(), temp_data_str, hj_crc);
        }
        else if(hj_crc > 16) //F
        {
             sprintf((char*)temp_hj_str, "##%s%s00%x", data_len_str.c_str(), temp_data_str, hj_crc);
        }
        else
        {
            sprintf((char*)temp_hj_str, "##%s%s000%x", data_len_str.c_str(), temp_data_str, hj_crc);
        }
        

        hj_data_packet = hj_data_packet + (char*)temp_hj_str;

        // cout << "hj212 packet:" << hj_data_packet << endl;


        //**************************************************私有协议封包************************************************
        //string len_j212_plus;
        int length= (strlen(hj_data_packet.c_str())+54);
        change(length);
        int num=len_j212.length();
        if(num==4)
        {

        }else if(num==3)
        {
            len_j212= "0" +len_j212;
        }else if(num==2)
        {
            len_j212="00" +len_j212;
        }else if(num==1)
        {
            len_j212="000"+len_j212;
        }
            //communicate_status
        sprintf((char*)send_data_packet,"$061%s%s00%s%s%s010%s%s%s%s@",id.c_str(),net_id.c_str(),len_j212.c_str(),Isr_mac.c_str(),mac.c_str(),accessdev_gps.c_str(),cpu_mem.c_str(),communicate_status,hj_data_packet.c_str());
       // sprintf((char*)send_data_packet,"$061%s%s00%s%s010%s%s@",id.c_str(),net_id.c_str(),len_j212.c_str(),mac.c_str(),accessdev_gps.c_str(),hj_data_packet.c_str());
        //sprintf((char*)hj_njdata_packet,"$061%s%s00%s%s010%s%s%s@",id.c_str(),net_id.c_str(),len_j212.c_str(),mac.c_str(),accessdev_gps.c_str(),hj_data_packet.c_str(),hj_njdata_rece);
        if(flag_nj==0){
            if(!recvnj_message.empty()){
                
                char temp[1500]={0};
                strcpy(temp,(char*)recvnj_message.front().c_str());
                sprintf((char*)hj_njdata_packet,"$061%s%s00%s%s%s010%s%s%s%s%s@",id.c_str(),net_id.c_str(),len_j212.c_str(),Isr_mac.c_str(),mac.c_str(),accessdev_gps.c_str(),cpu_mem.c_str(),communicate_status,hj_data_packet.c_str(),temp);
                //sprintf((char*)hj_njdata_packet,"$061%s%s00%s%s010%s%s%s@",id.c_str(),net_id.c_str(),len_j212.c_str(),mac.c_str(),accessdev_gps.c_str(),hj_data_packet.c_str(),temp);
                  //sprintf((char*)hj_njdata_packet,"$061%s%s00%s%s010%s%s@",id.c_str(),net_id.c_str(),len_j212.c_str(),mac.c_str(),accessdev_gps.c_str(),temp);

                recvnj_message.pop();
                flag_nj=1;          //如果还没有发送出去，就不在消息队列里面取收到的信息
                cout<<"《《《《《《《从消息队列中取出一条信息：等待发送》》》》》》》》》》》"<<endl;
            }
        }
        
           
        
        cout<<"获取数据成功"<<endl;
        len_private1= (strlen(send_data_packet));
        len_private2= (strlen(hj_njdata_packet));
        //len_private= i2hex(length);
        len_j212="";

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


/**
*   发送数据
*   @return
*/
void* trans_message(void* arg)
{
#ifdef DEBUG
    cout << "Thread: trans_message" << endl;
#endif 
    int succ_sendflag = 0;
    uint8_t check_buf[RX_SIZE];    //接收字符串
    int ret = -1;
    char in;
    char CONN[] = "CONN";
    uint8_t RX_buf[RX_SIZE];    //接收字符串
    string out_message = "";
    fd_bt = -1;
    fd_lan = -1;

    fd_set fdset;
   
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&fdset);
        FD_SET(fd_lora, &fdset);
        ret = select(fd_lora + 1, &fdset, NULL, NULL, NULL);
        if (ret > 0)
        {
            if (FD_ISSET(fd_lora, &fdset))
            {
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read(fd_lora, RX_buf, RX_SIZE);
                if (ret <= 58)
                {
                    string recv_message(RX_buf, RX_buf + ret);
                    string communicate_method;
                    if(protoc_03(recv_message,&communicate_method)==3)//确认收到的是03协议
                    {
                        pthread_mutex_lock(&mtx_sensor); //加锁，访问Is_
                        iS_getsensor=1;
                        pthread_mutex_unlock(&mtx_sensor);
                        pthread_cond_signal(&cond_sensor);//发出信号，唤醒读传感器数据的线程 
                        if(flag_nj==1)//收到南水所的数据，此时发送整个水土气数据包
                        {   
                            if (communicate_method.substr(0, 1) == "1") //使用lora发送
                            {
                                ret = write(fd_lora, hj_njdata_packet, strlen(hj_njdata_packet));
                                if(ret==0)
                                {
                                    continue;
                                }
                                if (ret < 0)
                                {
                                    cout << "fd_lora = " << fd_lora << endl;
                                    cout << "lora: send_data_packet = " << hj_njdata_packet << endl;
                                    cout << "lora: ret = " << ret << endl;
                                    cout << "Trans error(lora): " << strerror(errno) << endl;
                                }else
                                {
#ifdef DEBUG    
                                    cout << "hj_crc: " << hj_crc << endl;
                                    cout << "=======================" << endl;
                                    cout << "lora: send_data_packet = " << hj_njdata_packet << endl;
                                    cout << "Trans success(lora) " << endl;
                                    cout << "包的长度" <<len_private2<< endl;
#endif
                                    flag_nj=0;
                                    continue;
                                }

                            }else if (communicate_method.substr(1, 1) == "1")//使用wifi发送，修改通信模块类型
                            {
                                send_data_packet[3] ='2';
                                // ret = sendto(fd_wifi, hj_data_packet.c_str(), strlen(hj_data_packet.c_str()), 0, (struct sockaddr *)&wifi_ser_addr, sizeof(wifi_ser_addr));
                                ret = write(fd_wifi,hj_njdata_packet,strlen(hj_njdata_packet));
                                if(ret==0)
                                {
                                    continue;
                                }
                                if(ret <0)
                                {
 #ifdef DEBUG
                                    /*cout << "fd_wifi = " << fd_wifi << endl;
                                    cout << "wifi: send_data_packet = " << send_data_packet << endl;
                                    cout << "wifi: ret = " << ret << endl;
                                    cout << "Trans error(wifi): " << strerror(errno) << endl;
                                    cout << "包的长度" <<len_private<< endl;*/
 #endif
                                }else
                                {
#ifdef DEBUG
                                    cout << "hj_crc: " << hj_crc << endl;
                                    cout << "=======================" << endl;       
                                    cout << "fd_wifi = " << fd_wifi << endl;
                                    cout << "wifi: send_data_packet = " << hj_njdata_packet << endl;
                                    cout << "wifi: ret = " << ret << endl;
                                    cout << "Trans success(wifi) " << endl;
                                    cout << "包的长度" <<len_private2<< endl;
#endif
                                    flag_nj=0;
                                    continue;
                                }

                            }else if (communicate_method.substr(2, 1) == "1")//使用lan发送
                            {
                                continue;
                            }
                            else if (communicate_method.substr(3, 1) == "1")//使用bt发送
                            {
                                continue;
                            }
                        }else
                        {
                            if (communicate_method.substr(0, 1) == "1")//使用lora发送
                            {           
                                ret = write(fd_lora, send_data_packet, strlen(send_data_packet));
                                if(ret==0)
                                {
                                    continue;
                                }
                                if (ret < 0)
                                {
                                    cout << "fd_lora = " << fd_lora << endl;
                                    cout << "lora: send_data_packet = " << send_data_packet << endl;
                                    cout << "lora: ret = " << ret << endl;
                                    cout << "Trans error(lora): " << strerror(errno) << endl;
                                }
                                else
                                {
#ifdef DEBUG    
                                    cout << "hj_crc: " << hj_crc << endl;
                                    cout << "=======================" << endl;
                                    cout << "lora: send_data_packet = " << send_data_packet << endl;
                                    cout << "Trans success(lora) " << endl;
                                    cout << "包的长度" <<len_private1<< endl;
#endif
                                            
                                    continue;
                                }

                            }
                            else if (communicate_method.substr(1, 1) == "1")//使用wifi发送，修改通信模块类型
                            {
                                send_data_packet[3] ='2';
                                // ret = sendto(fd_wifi, hj_data_packet.c_str(), strlen(hj_data_packet.c_str()), 0, (struct sockaddr *)&wifi_ser_addr, sizeof(wifi_ser_addr));
                                ret = write(fd_wifi,send_data_packet,strlen(send_data_packet));
                                if(ret==0)
                                {
                                    continue;
                                }
                                if(ret <0)
                                {
 #ifdef DEBUG
                                    /*cout << "fd_wifi = " << fd_wifi << endl;
                                    cout << "wifi: send_data_packet = " << send_data_packet << endl;
                                    cout << "wifi: ret = " << ret << endl;
                                    cout << "Trans error(wifi): " << strerror(errno) << endl;
                                    cout << "包的长度" <<len_private<< endl;*/
 #endif
                                }
                                else
                                {

#ifdef DEBUG
                                    cout << "hj_crc: " << hj_crc << endl;
                                    cout << "=======================" << endl;       
                                    cout << "fd_wifi = " << fd_wifi << endl;
                                    cout << "wifi: send_data_packet = " << send_data_packet << endl;
                                    cout << "wifi: ret = " << ret << endl;
                                    cout << "Trans success(wifi) " << endl;
                                    cout << "包的长度" <<len_private1<< endl;
#endif
                                    continue;
                                }

                            }
                            else if (communicate_method.substr(2, 1) == "1")//使用lan发送
                            {
                                continue;
                            }
                            else if (communicate_method.substr(3, 1) == "1")//使用bt发送
                            {
                                continue;
                            }
                        }                              
                    }   
                }
            }
        }
    }
}


int main()
{
    system("./power_12v.sh on");
    sleep(1);
    pthread_t tid_getSensor,tid_getLan,tid_transMessage,tid_gps;

    pthread_create(&tid_gps,NULL,GET_GPS,NULL);

    sleep(1);
    int ret;                    //错误参数保存
    int fd_max;                 //最大文件描述符
    fd_set fdset;               //文件描述符集合
    int i, j;                   //临时变量，使用前注意初始化
    signal(SIGINT, stop);       //注册退出函数
    uint8_t RX_buf[RX_SIZE];    //接收字符串
    ret = dev_init();           //设备初始化，数组在进行调用传值的时候可以直接传递数组名
    if (-1 == ret) {
        printf("Init communicate device error!\n");
        return -1;
    }
    else
    {
        cout << "Init communicate device success" << endl;
        //设备初始化成功后，首先进行注册操作
        get_mac();              //首先获取接入设备的mac地址
        cpu_occupy=get_cpuOccupy();
        cout<<"cpu使用率："<<cpu_occupy<<endl;
        device_regist();        //设备注册:注册不成功的时候会一直卡在那里
                                //设备注册完成后，开启gps，数据采集，数据处理线程
        pthread_create(&tid_getSensor,NULL,get_sensor_data,NULL);
        //获取南水所的数据
        sleep(1);
        pthread_create(&tid_getLan,NULL,Get_L_data,NULL);
        sleep(1);
        pthread_create(&tid_transMessage,NULL,trans_message,NULL);
        pthread_join(tid_getSensor,NULL);
        pthread_join(tid_getLan,NULL);
        pthread_join(tid_gps,NULL);
        pthread_join(tid_transMessage,NULL);
    }
}
