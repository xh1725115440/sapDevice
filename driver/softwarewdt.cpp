#include "softwarewdt.h"
/*
openlog("mymodbus",LOG_PID,LOG_DAEMON);

syslog(LOG_ERR,"要写的信息")
syslog(LOG_INFO,"要写的信息")

close

*/
/*此代码是 CSoftwareWdt 类的构造函数，其主要功能为对类的成员变量进行初始化：
1.声明了一个局部变量 bId，用于后续的循环计数。
2.注释掉的 openlog 函数原本用于打开日志系统，这里暂时未启用。
3.借助 for 循环把 m_fReqWdtFlg 数组的每个元素初始化为 false。
4.运用 memset 函数将 m_wWdtTimeout、m_wWdtCount 和 m_wwdtName 数组初始化为 0。
*/
CSoftwareWdt::CSoftwareWdt()
{
	int bId;
 	//openlog("mymodbus",LOG_PID,LOG_DAEMON);  //打开日志系统
	for (bId = 0; bId < REQUEST_WDT_MAX_NUM; bId++)
		m_fReqWdtFlg[bId] = false;
	memset(m_wWdtTimeout,0, sizeof(m_wWdtTimeout));//void *memset(void *s, int c, size_t n);
	memset(m_wWdtCount,0, sizeof(m_wWdtCount)); //将指针 s 所指向的内存区域的前 n 个字节都设置为指定的值 c
	memset(m_wwdtName,0, sizeof(m_wwdtName));
}

CSoftwareWdt::~CSoftwareWdt()
{
	//closelog();
}
 
/*
* 功能:申请软件看门狗的ID
* 返回:申请失败返回-1，成功返回申请到ID
*
*/
int CSoftwareWdt::RequestSoftwareWdtID(const char *cThreadName, int wTimeout )
{
	int bId;
	int iRet = -1;
	for (bId = 0; bId < REQUEST_WDT_MAX_NUM; bId++)
	{
		if (m_fReqWdtFlg[bId] == false)	//等于0、表示该ID还没有被申请过,可以使用
		{
			m_wWdtTimeout[bId] = wTimeout;
			m_wWdtCount[bId] = 0;
			m_fReqWdtFlg[bId] = true;
			m_wwdtName[bId]=cThreadName;
			//syslog(LOG_ERR,"The Wdt ID=%d ,name=%s,request successful!", bId,m_wwdtName[bId]);
			printf("The thread %s request ID=%d successful. Timeout=%d\n", 
				cThreadName, bId, wTimeout);
			return bId;
		}
	}
 
	return iRet;
}
 
bool CSoftwareWdt::ReleaseSoftwareWdtID(const char *cThreadName, int wWdtId)
{
	bool fRet = false;
	
	if (wWdtId > REQUEST_WDT_MAX_NUM)
	{
		printf("The thread %s ID=%d is over MAX ID=%d\n", 
			cThreadName, wWdtId, REQUEST_WDT_MAX_NUM);
		return false;
	}
 
	if (m_fReqWdtFlg[wWdtId] != 0)
	{
		fRet = true;
		m_fReqWdtFlg[wWdtId] = false;
		m_wWdtTimeout[wWdtId] = 0;
		m_wWdtCount[wWdtId] = 0;
		printf("Release thread %s ID=%d\n", cThreadName, wWdtId);
	}	
 
	return fRet;
}
 
bool CSoftwareWdt::KeepSoftwareWdtAlive(int wWdtId)
{
	if (wWdtId > REQUEST_WDT_MAX_NUM)
		return -1;
 
	printf("start feed softdog Id=%d\n", wWdtId);
	m_wWdtCount[wWdtId] =0;
 
	return true;
}
 
int CSoftwareWdt::MontiorWdtRunState()
{
	int bId;
 	void* res;
	for (bId = 0; bId < REQUEST_WDT_MAX_NUM; bId++)
	{
		if (m_fReqWdtFlg[bId])
		{
			if (m_wWdtCount[bId]++ > m_wWdtTimeout[bId])
			{
				cout<<"计数器："<<m_wWdtCount[bId]<<endl;
				printf("The Wdt ID=%d is timeout\n", bId);// pthread_t tid_getSensor, tid_getLan, tid_transMessage, tid_gps, tid_ship_data, tid_softwd, tid_monitor485, tid_deviceRegist;
				cout<<m_wwdtName[bId]<<endl;
				if(strcmp(m_wwdtName[bId], "trans_message") == 0){ /* 新加的功能,与ISR之间的连接超时,解除对此线程的监控*/
					//先检测其线程是否存在
					int status = pthread_kill(tid_transMessage, 0);
					cout<<status<<endl;
					if(status == EINVAL) cout<<"wodecuo"<<endl;
					if(status == 0){//存在
						//this->ReleaseSoftwareWdtID(m_wwdtName[bId], bId);//解除对各个线程的监控
						for(bId = 0; bId < REQUEST_WDT_MAX_NUM; bId++){
							if (m_fReqWdtFlg[bId]){
								this->ReleaseSoftwareWdtID(m_wwdtName[bId], bId);
							}	
						}
						if(pthread_kill(tid_getSensor, 0) == 0) pthread_cancel(tid_getSensor) ;
						pthread_join(tid_getSensor, &res);
						if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   getSensor was cancelsed");

						//if(pthread_kill(tid_ship_data, 0) == 0) pthread_cancel(tid_ship_data) ;
						//pthread_join(tid_getSensor, &res);
						//if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   tid_ship_data was cancelsed");
						if(pthread_kill(tid_getLan, 0) == 0) pthread_cancel(tid_getLan) ;
						pthread_join(tid_getLan, &res);
						if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   tid_getLan was cancelsed");

						if(pthread_kill(tid_gps, 0) == 0) pthread_cancel(tid_gps) ;
						pthread_join(tid_gps, &res);
						if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   tid_gps was cancelsed");

						if(pthread_kill(tid_transMessage, 0) == 0) pthread_cancel(tid_transMessage) ;
						pthread_join(tid_transMessage, &res);
						if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   tid_transMessage was cancelsed");
						
					}else if(status == ESRCH){ //如果不存在,停止正在工作的其他线程
						if(pthread_kill(tid_getSensor, 0) == 0) pthread_cancel(tid_getSensor) ;
						pthread_join(tid_getSensor, &res);
						if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   getSensor was cancelsed");

						//if(pthread_kill(tid_ship_data, 0) == 0) pthread_cancel(tid_ship_data) ;
						//pthread_join(tid_ship_data, &res);
						//if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   tid_ship_data was cancelsed");

						if(pthread_kill(tid_getLan, 0) == 0) pthread_cancel(tid_getLan) ;
						pthread_join(tid_getLan, &res);
						if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   tid_getLan was cancelsed");

						if(pthread_kill(tid_gps, 0) == 0) pthread_cancel(tid_gps) ;
						pthread_join(tid_gps, &res);
						if(res == PTHREAD_CANCELED) printf("MontiorWdtRunState:   tid_gps was cancelsed");
					}		
				}
				//pid_t pt = getpid();
				return -1;
			}
		}
	}
	return 0;
}

/*定义了一个线程函数 softwarewd，其用途是对软件看门狗的运行状态进行监控。
*/
//即函数返回类型和参数类型都为 void*
void* softwarewd(void* arg)
{
	CSoftwareWdt *g_CsoftwareWdt=(CSoftwareWdt *)arg;
	//int fd=open("/dev/watchdog", O_WRONLY);
	//int ret = 0;

    //if (fd == -1) {
     //   perror("watchdog");
     //   close(fd);
     //   exit(EXIT_FAILURE);
     //   }
    // else{
     //	printf("hard watchdog open success!!!!!");
     //	}
	for(;;)//无限循环 for(;;) 持续监控软件看门狗的运行状态。
	{
		if(g_CsoftwareWdt->MontiorWdtRunState() == -1) {
			cout<<"线程有问题"<<endl;
			return NULL;
		}
		sleep(4);
        /*sleep(4);
        printf("循环监测线程一次\n");
        ret = write(fd, "\0", 1);
        if (ret != 1) 
        {
            ret = -1;
            continue;
        }
       printf("hard watchdog feed success!!!!!\n");*/
	}
}

