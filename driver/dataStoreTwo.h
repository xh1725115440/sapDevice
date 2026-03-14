#ifndef _DATA_H_
#define _DATA_H_
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h> //目录流头文件
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
using namespace std;
#define DIR_PERMS (S_IRWXU | S_IRWXG |S_IRWXO)  //定义的宏：给目录一定的权限 
//数据存储文件类；
//功能：只存储系统异常而导致未转发给ISR的数据，便于下次注册后发送
class dataStore
{
public:
	int creatDictory(const char* dictName);     //先检查当前目录是否存在，如若存在不创建返回-1，如若不存在，开始创建成功返回0
	int getFileName();	   //获取目录下的文件名
	bool writeToFile(const char *  buff);	//向文件中写入数据
	bool readFile(char**  buff);
	dataStore();
	~dataStore();
private:
	char* dictoryName; //文件存储的绝对路径名
	char* fileName;
	size_t myGetLine(char** buffer, size_t* num, FILE* sourstream);
};
#endif
