#include "communicaManage.h"
using namespace std;



communicaManage::communicaManage()
{
	communicateNode = new struct communicateNode;
	communicateNode->com = NULL;
	communicateNode->enabled = false;
	communicateNode->id = -1;
	//communicateNode->id = 0;
	communicateNode->next = NULL;
	//communicateNode->myself = nullptr;
}

communicaManage::~communicaManage()
{
	delete communicateNode;
}

bool communicaManage:: cmp(vector<int>& a, vector<int>& b)
{
	return a[0] < b[0];
}
 struct communicateNode* communicaManage::findcommunicate(int id)  //查找是否存在此ID号的通信设备
{
	struct communicateNode *temp = communicateNode;	
	while (temp!= NULL) //遍历完链表
	{	
		if (temp->id == id )
		{
			cout << "找到通信设备,ID:"<<temp->id << endl;
			return temp;
		}
		temp = temp->next;
	}
	cout << "未找到你查找的通信设备" << endl;
	return temp;
}

bool communicaManage::addCommunicateNode(int id, int fd, int timeout) //向链表里面加入新初始化成功的设备节点
{
	struct communicateNode *temp=NULL;// = new struct communicateNode;
	struct communicate * com = createComnode(fd,timeout);
	if (communicateNode->com == NULL) //当前没有通信设备加入,且为第一个初始化成功的通信设备
	{
		communicateNode->com = com;
		communicateNode->id = id;
		communicateNode->enabled = false;
		communicateNode->next = NULL;
		cout << "添加设备ID:"<<id<<"成功" << endl;
		return true;
	}
	else 
	{
		temp = communicateNode;
		while (temp != NULL)
		{
			if (temp->id == id) {
				cout << "添加节点失败，已经存在ID为：" << id << "的设备" << endl;
				return false;
			}
			if (temp->next == NULL) // 遍历到末尾一个节点
				break;
			temp = temp->next;
		}
		struct communicateNode *temp1 = new struct communicateNode;
		temp1->com = com;
		temp1->id = id;
		temp1->enabled = false;
		temp1->next = NULL;
		temp->next = temp1;
		cout << "添加设备ID:" << id << "成功" << endl;
		return true;
	}
}

bool communicaManage::addCommunicateNode(int id)    //添加一个空设备节点，一般不使用
{	
	struct communicateNode *temp = NULL;
	struct communicate * com = createComnode();
	if (communicateNode->com == NULL) //当前没有通信设备加入,且为第一个初始化成功的通信设备
	{
		communicateNode->com = com;
		communicateNode->id = id;
		communicateNode->enabled = false;
		communicateNode->next = NULL;
		cout << "添加设备ID:" << id << "成功" << endl;
		return true;
	}
	else
	{
		temp = communicateNode;
		while (temp != NULL) //遍历到末尾一个节点
		{
			if ((temp->id == id) )//已经添加过此ID的通信设备
			{
				cout << "添加节点失败，已经存在ID为：" << id << "的设备" << endl;
				return false;
			}
			if (temp->next == NULL) // 遍历到末尾一个节点
				break;
			temp = temp->next;
			
		}
		struct communicateNode *temp1 = new struct communicateNode;
		temp1->com = com;
		temp1->id = id;
		temp1->enabled = false;
		temp1->next = NULL;
		temp->next = temp1;
		cout << "添加设备ID:" << id << "成功" << endl;
		return true;
	}
}

bool communicaManage::addCommunicateNode(int id,int fd)  //常用此接口函数
{
	struct communicateNode *temp = NULL;// = new struct communicateNode;
	struct communicate * com = createComnode(fd);
	if (communicateNode->com == NULL) //当前没有通信设备加入,且为第一个初始化成功的通信设备
	{
		communicateNode->com = com;
		communicateNode->id = id;
		communicateNode->enabled = false;
		communicateNode->next = NULL;
		cout << "添加设备ID:" << id << "成功" << endl;
		return true;
	}
	else {
		temp = communicateNode;
		while (temp != NULL) //遍历到末尾一个节点
		{	
			if ((temp->id == id) )//已经加入了的设备，未被删除
			{
				cout << "添加节点失败，已经存在ID为：" << id << "的设备" << endl;
				return false;
			}
			if (temp->next == NULL)
				break;
			temp = temp->next;
		}
		struct communicateNode *temp1 = new struct communicateNode;
		temp1->com = com;
		temp1->id = id;
		temp1->enabled = false;
		temp1->next = NULL;
		temp->next = temp1;
		cout << "添加设备ID:" << id << "成功" << endl;
		return true;
	}
}

bool communicaManage::deletecommunicateNode(int id)  //删除指定ID的通信设备节点
{
	//定义一个虚拟头节点
	struct communicateNode *head = new struct communicateNode(-1, communicateNode);
	while (head->next!=NULL) 
	{
		if (head->next->id == id)  //找到要删除的通信设备节点
		{
			struct communicateNode *temp = head->next;
			head->next = head->next->next;
			delete temp;
			communicateNode = head->next;
			delete head;
			cout << "删除成功" << endl;
			return true;
		}
		else
		{
			head = head->next;
		}
	}
	return false;
}

bool communicaManage::reinit(int id ,int para)			//参数para根据回调函数实际情况，无参数则默认置0
{	 
	struct communicateNode* temp = findcommunicate(id);
	if (temp == NULL)
	{
		cout << "当前没有对应的通信设备,初始化失败" << endl;
		return false;
	}
	if (temp->com->reback[0] == NULL) {
		cout << "重新初始化失败：此通信设备为注册初始化函数，请先添加再重初始化" << endl;
		return false;
	}
	else
	{
		int fd = temp->com->reback[0](para);				//执行重新初始化操作，并拿到初始化后新的设备描述符fd
		int(*func)(int para) = temp->com->reback[0];		//保存此设备的回调函数
		bool ret = deletecommunicateNode(id);			//重新初始化之后，先对此通信设备的删除
		addCommunicateNode(id, fd);						//再重新加入链表
		callbackRgist(id,0, func);						//重新加入回调函数
		cout << "重新初始化成功" << endl;
		return true;
	}
}                                                         

bool communicaManage::callbackRgist(int id,int pos ,int(*function)(int)) //为外部实现的一个添加对应操作函数的接口
{ 
	if (pos > 2) return false;
	struct communicateNode* temp = findcommunicate(id);
	if (temp==NULL) 
	{
		cout << "当前没有对应的通信设备,回调函数注册失败" << endl;
		return false;
	}
	else 
	{
		temp->com->reback[pos] = function; //储存每个初始化成功的设备的初始化函数
		cout << "回调函数注册成功" << endl;
		return true;
	}
}
//创建一个空设备节点
communicate * communicaManage::createComnode()
{
	communicate * com = new communicate;
	com->fd = -1;
	for (int i = 0; i < 3; i++)
	{
		com->reback[i] = NULL;
	}
	com->timecount = -1;
	com->timeout = -1;
	return com;
}
//创建一个设备节点  fd文件描述符
struct communicate* communicaManage::createComnode(int fd)
{
	communicate * com = new communicate;
	com->fd = fd;
	for (int i = 0; i < 3;i++) 
	{
		com->reback[i] = NULL;
	}
	com->timecount = -1;
	com->timeout = -1;
	return com;
}


struct communicate* communicaManage::createComnode(int fd,int timeout)
{
	communicate * com = new communicate;
	com->fd = fd;
	for (int i = 0; i < 3; i++)
	{
		com->reback[i] = NULL;
	}
	com->timecount = -1;
	com->timeout = timeout;
	return com;
}

int communicaManage::getSingleFd(int id)  //获取单个通信设备对应的fd号
{

	struct communicateNode* com=findcommunicate(id);
	if (com == NULL)  return -1;
	return com->com->fd;
}

vector<int> communicaManage::getAllFd()  //获取设备管理类所有的已初始化成功设备的fd号
{	
	vector<int> result;
	struct communicateNode * temp = communicateNode;
	while (temp != NULL)
	{
		result.push_back(temp->com->fd);
		temp = temp->next;
	}
	return result;
}
/*  获取全部通信设备id号、fd号 及按照id号从大到小顺序返回
*/
vector<vector<int>>  communicaManage::getALLIfd()  //返回按照ID从小到大的顺序动态数组
{
	std::vector<vector<int>> result;
	std::vector<int> vec;
	struct communicateNode * temp = communicateNode;
	while (temp != NULL)
	{
		vec.push_back(temp->id);
		vec.push_back(temp->com->fd);
		result.push_back(vec);
		vec.clear();
		temp = temp->next;
	}
	sort(result.begin(), result.end(), cmp);//cmp 是一个自定义的比较函数，用于指定排序的规则
	return result;
}

int communicaManage::getSuccessFd()
{
	struct communicateNode* temp = communicateNode;
	while (temp!=NULL)
	{
		if (temp->enabled == true)
		{
			return temp->com->fd;
		}
		temp = temp->next;
	}
	return -1;
}
int communicaManage::getSuccessId()
{
	struct communicateNode* temp = communicateNode;
	while (temp!=NULL)
	{
		if (temp->enabled == true)
		{
			return temp->id;
		}
		temp = temp->next;
	}
	return 0;
}
bool communicaManage::setEnable(int id)
{	
	struct communicateNode* temp = NULL;	
	if ((temp=findcommunicate(id)) != NULL)
	{
		temp->enabled = true;
		return true;
	}
	return false;
}

int communicaManage::getSize()			 //获取设备管理类管理设备的大小
{
	int size=0;
	struct communicateNode * temp= communicateNode;
	while (temp != NULL)
	{
		size++;
		temp = temp->next;	
	}
	return size;
}

bool communicaManage::isExist(int id)
{	
	struct communicateNode * temp = communicateNode;
	while (temp != NULL) {
		if (temp->id == id) return true;
		temp = temp->next;
	}
	return false;
}








