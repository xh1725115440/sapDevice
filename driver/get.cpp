#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <iostream>   
#include <unistd.h>
#include "get.h"

using namespace std;  
  
void get_memoccupy(MEM_OCCUPY *mem) //对无类型get函数含有一个形参结构体类弄的指针O  
{  
    FILE *fd;  
    char buff[256];  
    MEM_OCCUPY *m;  
    m = mem;     
    fd = fopen("/proc/meminfo", "r");  
    //MemTotal: 515164 kB  
    //MemFree: 7348 kB  
    //Buffers: 7892 kB  
    //Cached: 241852  kB  
    //SwapCached: 0 kB  
    //从fd文件中读取长度为buff的字符串再存到起始地址为buff这个空间里   
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %lu ", m->name1, &m->MemTotal);  
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %lu ", m->name2, &m->MemFree);  
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %lu ", m->name3, &m->Buffers);  
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %lu ", m->name4, &m->Cached);  
    fgets(buff, sizeof(buff), fd);   
    sscanf(buff, "%s %lu", m->name5, &m->SwapCached);       
    fclose(fd);     //关闭文件fd  
}  
  
  
int get_cpuoccupy(CPU_OCCUPY *cpust) //对无类型get函数含有一个形参结构体类弄的指针O  
{  
    FILE *fd;  
    char buff[256];  
    CPU_OCCUPY *cpu_occupy;  
    cpu_occupy = cpust;       
    fd = fopen("/proc/stat", "r");  
    fgets(buff, sizeof(buff), fd);  
    sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle, &cpu_occupy->lowait, &cpu_occupy->irq, &cpu_occupy->softirq);            
    fclose(fd);     
    return 0;  
}  
string cal_memoccupy(MEM_OCCUPY *m)
{
	double mem_use = 0;
	int mem_d;
	string mem;
	mem_use=m->MemFree * 1.0 / (m->MemTotal * 1.0);
	mem_d = mem_use * 100;
	if (mem_d > 0) {
		if (mem_d < 10)
		{
			string str1 = to_string(mem_d);
			mem = "0" + str1;
		}
		else if (10 <= mem_d < 100) {
			string str2 = to_string(mem_d);
			mem = str2;
		}
		else {
			mem = "99";
		}
	}
	else {
		mem = "01";
	}
	return mem;
}
  
string cal_cpuoccupy(CPU_OCCUPY *o, CPU_OCCUPY *n)  
{  
    unsigned long od, nd;  
    double cpu_use = 0;  
	int cpu_d;
	string cpu_r;
    od = (unsigned long)(o->user + o->nice + o->system + o->idle + o->lowait + o->irq + o->softirq);//第一次(用户+优先级+系统+空闲)的时间再赋给od  
    nd = (unsigned long)(n->user + n->nice + n->system + n->idle + n->lowait + n->irq + n->softirq);//第二次(用户+优先级+系统+空闲)的时间再赋给od  
    double sum = nd - od;  
    double idle = n->idle - o->idle;  
    cpu_use = idle / sum;  
    idle = n->user + n->system + n->nice - o->user - o->system - o->nice;  
    cpu_use = idle / sum; 
	cpu_d = cpu_use * 100;

	if (cpu_d>0) {
		if (cpu_d < 10)
		{
			string str1 = to_string(cpu_d);
			cpu_r = "0" + str1;
		}
		else if (10 <= cpu_d && cpu_d<100) {
			string str2 = to_string(cpu_d);
			cpu_r = str2;

		}
		else {
			cpu_r = "99";
		}
	}
	else {
		cpu_r = "01";
	}
	return cpu_r;
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
    mem_cpu=mem+cpu;
    cpu_mem=cpu+mem;
    return cpu_mem;
} 
/*int main( int argc, char **argv )  
{  
    MEM_OCCUPY mem_stat;  
    CPU_OCCUPY cpu_stat1;  
    CPU_OCCUPY cpu_stat2;
	int mem_use;
	string mem;
	int cpu_use;
	string cpu;
    //获取内存
    //(MemTotal - MemFree)/ MemTotal
    get_memoccupy((MEM_OCCUPY *)&mem_stat);  
	mem = cal_memoccupy((MEM_OCCUPY *)&mem_stat);

	cout << mem << endl;
    //第一次获取cpu使用情况  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);  
    usleep(100000);  
    //第二次获取cpu使用情况  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);  
    //计算cpu使用率  
	cpu =cal_cpuoccupy((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);

	cout << cpu << endl;
    return 0;
}*/