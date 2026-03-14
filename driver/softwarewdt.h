#pragma once
#ifndef _SOFTWAREWDT_H_    
#define _SOFTWAREWDT_H_ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <syslog.h> //写系统日志服务的头文件
#include <iostream>
#include "protocol_process.h"
#include <pthread.h>
#include <signal.h>
using namespace std;
/*
openlog("mymodbus",LOG_PID,LOG_DAEMON);

syslog(LOG_ERR,"要写的信息")
syslog(LOG_INFO,"要写的信息")

close
*/
#define REQUEST_WDT_MAX_NUM	20
extern pthread_t tid_getSensor, tid_getLan, tid_transMessage, tid_gps, tid_ship_data, tid_softwd, tid_monitor485, tid_deviceRegist;
class CSoftwareWdt 
{
private:
	bool m_fReqWdtFlg[REQUEST_WDT_MAX_NUM];	//=true 已经申请ID成功，=false反之
 
	int m_wWdtTimeout[REQUEST_WDT_MAX_NUM]; //计数器，如若超过这个时间系统重启
 
	int m_wWdtCount[REQUEST_WDT_MAX_NUM];

	const char* m_wwdtName[REQUEST_WDT_MAX_NUM];   //维护每个被监测的线程的名字
 
public:
	CSoftwareWdt();
 
	~CSoftwareWdt();
 
	int RequestSoftwareWdtID(const char *cThreadName, int wTimeout);	
 
	bool ReleaseSoftwareWdtID(const char *cThreadName, int wTimeout);
 
	bool KeepSoftwareWdtAlive(int wWdtId);
 
	int MontiorWdtRunState();
};

void* softwarewd(void* arg);
//void* hardwd(void* arg);
#endif
//void stop(int signum);