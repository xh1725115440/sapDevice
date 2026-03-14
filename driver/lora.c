#include <stdio.h>
#include "lora.h"
#include "usbctl.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <string>
#include "usbctl.h"
#include <iostream>
using namespace std;
/*  打开lora  成功返回文件描述符
*/
int lora_open()
{
    int lora;  //文件描述符
    lora = open_port(0);//打开lora
    if(-1 == lora){         //lora设备打开失败
        return -1;
    }
    else{
        set_opt(lora, 115200, 8, 'n', 1);  //文件描述符，波特率，数据位，校验位，停止位
    }
    return lora;
}

int RS485_open()
{
	int rs=open_port(4);
	if(-1 == rs){         //lora设备打开失败
        return -1;
    }
    else{
        set_opt(rs, 9600, 8, 'n', 1);
    }
    return rs;
}


/*void RS485_Monitor(void *arg){
	char buf[1024];
	int fdRs485=RS485_open();
	struct timeval timeout;
	timeout.tv_sec = 60;
    timeout.tv_usec = 0;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fdRs485,&fdset);
    int ret;
    while(1){
    	FD_ZERO(&fdset);
    	cout<<"文件描述符："<<fdRs485<<endl;
    	cout<<"RS485串口开始监听"<<endl; 
    }
}*/