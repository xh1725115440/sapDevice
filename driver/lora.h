#pragma once        //使得头文件最多被包含一次

#ifndef _LORA_H_    //ifndef 为了避免重定义，虽然ifdef在c中可以避免，但是在c++中，ifdef重定义还是会报错
#define _LORA_H_    //使用_LORA_H_作为标识，当其未被定义，则lora_open()函数也未申明

int lora_open();
//void RS485_Monitor(void *arg);
//void RS485_Monitor_Plus(void *arg);
#endif


