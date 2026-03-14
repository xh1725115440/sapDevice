#pragma once
#ifndef _USART_H_    
#define _USART_H_  
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <string>
#include "usbctl.h"
#include <iostream>
#include <queue>
#include "softwarewdt.h"
#define RS485_A                 "100"     
#define RS485_B                 "010"  
#define RS485_C                 "001"   
#define RS232                   "101"
#define RJ45                    "111"       

void* usart_monitor(void* parm);
/*485串口监听函数*/
void* RS485_Monitor(void *arg);

/*用于给RS485_Monitor线程函数传递参数所设置的结构体*/
struct monitor485
{
	queue<string> *que;
	CSoftwareWdt *g_CsoftwareWdt;
	monitor485(queue<string> *que, CSoftwareWdt *g_CsoftwareWdt) : que(que), g_CsoftwareWdt(g_CsoftwareWdt) {};
};
#endif