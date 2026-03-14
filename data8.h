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
#include <string>
#include <time.h>
//#define _GNU_SOURCE
#define DICTORYNAME  "/home/root/mydata"
#define CONFDICTNAME "/home/root/myconf"
#define ROW         "ROW"
#define POINTER     "POINTER"
#define ALREAD      "ALREAD"
#define MAXSIZE     30
#define MAXROW      3
#define MODE_CREAT int 
#define MODE_TXT    1
#define MODE_CONF    2
#define EXCUSUCEE    1
#define CREATSUCEE   0
#define ERROCCURCE  -1
#define CREATERR		-2 
#define DICTEXIST	-3
#define DICTNEXISET -4
#define FILEEXIST	-5
#define FILENEXIST  -6
#define CONFEXIST   -7
#define CONFNEXIST   -8
#define EXCUFALSE   -9
#define FILEISFULL  -10
#define FILEREADEND -11
#define WRITEFINSH  -12
using namespace std;
#define DIR_PERMS (S_IRWXU | S_IRWXG |S_IRWXO)  //定义的宏：给目录一定的权限 
//数据存储文件类；
//功能：只存储系统异常而导致未转发给ISR的数据，便于下次注册后发送
struct myFileData {
	char *fileName[MAXSIZE];						//存储每个数据文本及其对应配置文件名字
	char *confName[MAXSIZE];
	int enable[MAXSIZE];
};
class dataStore
{
public:
	//bool isExist(const char* dictName, const char* confName);
	int creatDictory(const char* dictName);
	int creatDictory(const char* dictName, const char* confName);    //创建数据文本文件夹及其配置文件夹，如若存在不创建
	int getFileName();												//获取目录下的文件名
	int getFileName2();												//获取目录下的文件名2
	int getConfName(const char* fileName, char** rebuff);		 	//获取fileName相对应的配置文件
	int writeToFile(const char *  buff);							//向文件中写入数据
	int readFile(char**  buff);
	int creatFile(const char* FileName, MODE_CREAT mode);				//创建文件，参数为将要创建的名字 mode: 1---创建数据文本文件；2---创建相对应的配置文件
	dataStore();
	~dataStore();
private:
	char* dictoryName;						//文本文件夹存储的绝对路径名
	char* confDictName;						//配置文件夹的绝对路径名
	int  getEnableFile();					//同一时刻获取一个文件进行操作
	char* fileNameEn;						//可进行操作的数据文本名字
	char* confNameEn;						//可进行操作的配置文本名字
	char* fileNameENr;
	char* newFileName;						//因为行数超出新建的数据文本名字
	char* newConfName;						//因为行数超出新建的配置文本名字
	size_t myGetLine(char** buffer, size_t* num, FILE* sourstream);
	myFileData *mydata = NULL;
	int upConfFile(const char* confName, const char* filed, int filedValue);
	int getOfConf(const char* confName, const char* filed);
};
#endif



