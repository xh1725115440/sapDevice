#ifndef _GET_H_
#define _GET_H_

#pragma once
#include <iostream>
#include <string>
#include <sstream>
using namespace std;
typedef struct MEMPACKED         //定义一个mem occupy的结构体  
{  
    char name1[20];      //定义一个char类型的数组名name有20个元素  
    unsigned long MemTotal;  
    char name2[20];  
    unsigned long MemFree;  
    char name3[20];  
    unsigned long Buffers;  
    char name4[20];  
    unsigned long Cached;  
    char name5[20];  
    unsigned long SwapCached;  
}MEM_OCCUPY; 

typedef struct CPUPACKED         //定义一个cpu occupy的结构体  
{  
    char name[20];      //定义一个char类型的数组名name有20个元素  
    unsigned int user; //定义一个无符号的int类型的user  
    unsigned int nice; //定义一个无符号的int类型的nice  
    unsigned int system;//定义一个无符号的int类型的system  
    unsigned int idle; //定义一个无符号的int类型的idle  
    unsigned int lowait;  
    unsigned int irq;  
    unsigned int softirq;  
}CPU_OCCUPY;


void get_memoccupy(MEM_OCCUPY *mem);
int get_cpuoccupy(CPU_OCCUPY *cpust);
string cal_memoccupy(MEM_OCCUPY *m);
string cal_cpuoccupy(CPU_OCCUPY *o, CPU_OCCUPY *n);
string get_cpuOccupy();

#endif
