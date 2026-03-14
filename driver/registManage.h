#pragma once
#ifndef _REGISTMANAGE_H_    
#define _REGISTMANAGE_H_ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include "softwarewdt.h" 
#include "communicaManage.h" 
using namespace std; 
#define RX_SIZE                 58                      //最大协议包字节数
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
#define TIME_SEND               "20"                    //时间戳下发
extern string  id, net_id, mac, cpu_occupy, gps_lon, gps_lat, Isr_mac, current_time;
extern char communicate_status[];
extern int monitor_time;
extern communicaManage* CM;
/*
结构体参数：
定义两个成员：（1）线程监管全局类 （2）初始化成功的通信设备列表
功能：为注册线程函数传参
*/
struct deviceRegist
{
	CSoftwareWdt *g_CsoftwareWdt;
	vector<vector<int>> *vec;
	deviceRegist(CSoftwareWdt *g_CsoftwareWdt,vector<vector<int>> *vec) :g_CsoftwareWdt(g_CsoftwareWdt), vec(vec) {};  //初始化列表
};

/*
函数功能：通信设备的注册
参数arg：一个结构体包含：一个线程监管的结构体，一个初始化成功的二维数组
*/
void* device_regist(void *arg) ;	//SAP的注册

void* device_registCsmaCd(void *arg);
#endif