#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>          /* See NOTES */
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <iomanip>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>
#include "communicaManage.h"
#include "get.h"
#include <queue>
#include <string>
using namespace std;

//string dev_register(string buf, string *num, string net);

string switch10_16(int num);
int change(long long int num, string& str);
int protoc_17(string buff);
int protoc_03(const string buff,string *communicate_method);
float int2pString(int num);
string packet06(string strT, string portInfo);

void packet20(string& strPacket);//向ISR请求时间戳
void stop(int signum);
void mytest(string buffer);
int dataTofile(queue<string>& buffer);
int readFromFile(queue<string>& recvQueue);