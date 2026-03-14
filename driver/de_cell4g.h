// Made by Yang.
// 2019/4/19
#ifndef __DE_CELL4G_H_
#define __DE_CELL4G_H_


#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>

int cell4g_detect(char* net_name);
int cell4g_ip_port_check(char* remote_ip, int remote_port);
int cell4g_open(void);
int hard4g_init(char* net_name);


#endif
