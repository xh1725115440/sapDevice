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
#define REQ_DATA                "03"                    //数据请求
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
int 