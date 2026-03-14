#pragma once
#ifndef _COMMUNICAMANAGE_H_    
#define _COMMUNICAMANAGE_H_
#include<vector>
#include<algorithm>
using namespace std;

#include<iostream>
																/*通信方式管理类，管理各个通信设备的初始化、发送消息等*/
#define communicatemax 4
struct communicate {											//每个初始化成功后通信设备的拥有的结构体对象
	int fd;														//通信设备描述符
	//数组reback 包含三个元素 每个元素都是一个指向返回值为int类型、参数为int类型的函数指针
	int (*reback[3])(int fd);                                   //用来存储各个函数的入口地址，完成重新初始化、发送消息等 ：
																//在这里固定为 0：初始化  1：发操作  2：用户定义的其他操作 	
	int timecount;												//生存计数器
	int timeout; 												//超数阈值，如若超过阈值，就认定为此通信设备连接中断，即开始重新初始化
};
struct communicateNode {
	struct communicate* com;    /*最新的通信设备*/
	int id;											//固定位Lora 0  wifi 1  blueteeth 2  lan 3 ，后续根据时间情况可扩展且可自由分配 
	bool enabled;									//使能位，是否通过此通信节点注册成功
	struct communicateNode* next;                   //链表结构，指向下一个节点的指针
	communicateNode(int id, communicateNode *next) : id(id), next(next) {}  //有参构造
	communicateNode(){}
	//struct communicateNode*	myself;							//后期可以考虑在结构体上维护一个指向自己的指针
};
class communicaManage
{	
public:
	//struct communicate comstatus[communicatemax];				//固定位Lora 0  wifi 1  blueteeth 2  lan 3 ，后续根据时间情况可扩展 
	bool addCommunicateNode(int id);			                //添一个通信设备节点，且当前通信设备未被初始化成功
	bool addCommunicateNode(int id, int fd);
	bool addCommunicateNode(int id, int fd, int timeout);
	bool deletecommunicateNode(int id);							//删除已经添加的通信设备
	bool reinit(int id,int para);								//执行对应通信设备节点初始化
	bool callbackRgist(int id,int pos, int(*function)(int));	//回调函数的注册
	int  getSingleFd(int id);									//获取单个通信设备对应的fd号
	vector<int> getAllFd();										//获取全部通信设备对应的fd号
	vector<vector<int>>  getALLIfd();	//函数不接受参数						//获取全部通信设备id号、fd号及按照id号从大到小顺序返回
	int getSuccessFd();
	int getSuccessId();
	bool setEnble(int id);										//将注册成功的通信设备描述置为使能
	int getSize();
	bool isExist(int id);
	communicaManage();
	~communicaManage();
private:
	struct communicateNode *communicateNode = NULL;
	struct communicateNode* findcommunicate(int id);

	struct communicate* createComnode();   //成员函数 返回值为struct communicate*
	struct communicate* createComnode(int fd);					//节点的初始化
	struct communicate* createComnode(int fd, int timeout);

	static bool cmp(vector<int>& a, vector<int>& b);

};
#endif