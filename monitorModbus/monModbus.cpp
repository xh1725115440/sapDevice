#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/inotify.h>
#include <limits.h>
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <iostream>

#define pathname "/home/root/myWatch.txt"
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
using namespace std;

/* 从配置文件获取进程号*/
int getMyPid(const char * pathname1){
	int fd = open(pathname, O_RDONLY | O_NOATIME);
	if(fd < 0){
		perror("main open :");
	}
	char buffer[50];
	if(read(fd, buffer, 50) == -1){
		perror("错误提示：");
	}
	close(fd);
	return atoi(buffer);
}

void* hardDogWatch(void* arg){

	int fd =open("/dev/watchdog", O_WRONLY);
	int ret = 0;
    if (fd == -1) {
    	perror("watchdog");
    	close(fd);
        exit(EXIT_FAILURE);
    }
    else{
     	printf("hard watchdog open success!!!!!");
    }
	for(;;)
	for(;;)
	{
		sleep(8);
		int ptt = getMyPid(pathname);//获取进程ID号
		cout << " 监控的进程号: "<< (ptt) <<endl;
		ret = write(fd, "\0", 1);
        if (ret != 1) 
        {
            ret = -1;
            continue;
        }
       printf("hard watchdog feed success!!!!!\n");
		if(ptt == 0){ // 检测到sap初始化失败，进程退出，myWatch.txt写入0
			cout << " 正常重启 1: "<<endl;
			sleep(15);
			system("./modbus_6001 &");
			continue;
		} 
        if(kill(ptt, 0) == -1 && errno == ESRCH){ //监测进程是否存在，如果不存在就重启
        	cout << " 异常重启 2: "<<endl;
        	sleep(15);
        	system("./modbus_6001 &");
        }    
	}
}





int main(){  
	/*int pt = getMyPid(pathname); //不能防止进程异常退出（只能监控正常退出）
	cout<<"当前监控的进程号为： "<<pt<<endl;
	int inotifyFd ;
	inotifyFd = inotify_init(); //初始化一个监控的实例
	int fdWatch = inotify_add_watch(inotifyFd, pathname, IN_MODIFY);//将要监控的文件加入此实例中
	if(fdWatch < 0) perror("错误原因：");
	struct inotify_event *event;
	fd_set fdset;
	char buf[BUF_LEN];
	ssize_t numRead;
	pthread_t tid;
	pthread_create(&tid, NULL, hardDogWatch, &pt);
	for(;;){
		sleep(10);
    	FD_ZERO(&fdset);
    	FD_SET(inotifyFd, &fdset);
        int ret = select(inotifyFd + 1, &fdset, NULL, NULL, NULL);
        if(ret > 0){
        	if (FD_ISSET(inotifyFd, &fdset)){
        		numRead = read(inotifyFd, buf, BUF_LEN);
        		if(numRead == -1) perror("read: ");
        		event = (struct inotify_event *) buf;
        		if(event->mask & IN_MODIFY){ //文件被修改，说明modus进程异常,此时查看进程号是否被修改
        			cout<<"MODBUS异常"<<endl;
        			sleep(3);
        			int ptt = getMyPid(pathname);
        			cout<<"当前进程号 ptt : "<<ptt <<endl;
        			if(ptt == 0) {
        				cout<<"modbus进程已经退出，准备重启modbus进程"<<endl;
        				system("./modbus &");
        			}else if(ptt != 0) { //modbus进程被重启的时候，修改文件
        				cout<<"modbus 进程已经被重启"<<endl;
        				pt = ptt; 
        			} 

        			if(kill(pt, 0) == -1 && errno == ESRCH){ //要监控的进程不存在，重启此进程
						//重启进程
						cout<<"再次确认：	MODBUS不存在"<<endl;
					}else if(kill(pt, 0) == 0){
						cout<<"再次确认： modbus重启成功"<<endl;
					}	
        		}
        	}
        }
	}*/
	pthread_t tid;
	pthread_create(&tid, NULL, hardDogWatch, NULL);
	pthread_join(tid, NULL);
	return 0;
}