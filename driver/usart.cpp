#include "usart.h"
#include "protocol_process.h"
/*
void* usart_monitor(void* parm){
	int fd, ret;
    char *p;
    char buf[512];
    // select 方式
    struct timeval timeout;
    fd_set fdset;
    FD_ZERO(&fdset);

    // 遍历com口
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int i = 5;
    while(i > 0)
    {
        fd = open("/dev/tong_bt", O_RDWR | O_NOCTTY | O_NDELAY); //打开要监听的串口
        if (-1 == fd)
        {                                                                                                     
            i--;
            continue;
        }
        else{
            break;
        }
    }
    if(fd != -1)
    {
        // 配置USB端口
        ret = set_opt(fd, 115200, 8, 'n', 1);
        FD_SET(fd,&fdset);
        // 检测设备启动
        // 监听事件 1秒钟 超时跳过
        ret = select(fd + 1, &fdset, NULL, NULL, &timeout);
        switch (ret)
        {    
        case -1:                                  //错误
            printf("->> Error occurs when select() \n");
            break;
        case 0:
            printf("->> Select() timeout \n");        //超时
            break;
        default:
            if (FD_ISSET(fd, &fdset))
            {
                ret = read_port(fd, buf, 60);
                if (ret > 0)
                {
                    // 检查AT指令返回值
                    p = strstr(buf, "OK");
                    if (p != NULL)
                    {
                        //设备重启成功
                        ret = write_port(fd, OPEN_VISUAL, strlen((const char*)OPEN_VISUAL));
                        sleep(1);
                        ret = write_port(fd, AT_SCAN, strlen((const char*)AT_SCAN));
                        //开始扫描
                        while(1)
                        {
                            FD_ZERO(&fdset);
                            FD_SET(fd, &fdset);
                            ret = select(fd + 1, &fdset, NULL, NULL, NULL);
                            if(ret > 0)
                            {
                                if(read(fd, &in, 1) == 0)continue;
                                if(in != '\n')
                                {
                                    //判断不是换行符
                                   pro_message = pro_message + in; 
                                }
                                else
                                {
                                    //一行末尾调用process_message函数
                                    //cout << "pro_message: " << pro_message << endl;
                                    int ret_proc_mess = blue_proc_message(pro_message);
                                    if(ret_proc_mess == 1)
                                    {
                                        //返回值为1表示还没有到末尾，且没有查询到
                                        pro_message = "";
                                        continue;
                                    }
                                    else if(ret_proc_mess == 2)
                                    {
                                        //返回值为2表示还没有到末尾，且查询到了
                                        device_mac = pro_message;
                                        cout << "g2020: " << device_mac << endl;
                                        return fd;
                                    }
                                    else if(ret_proc_mess == 3)
                                    {
                                        //查询到了末尾，且没有查询到，则退出
                                        FD_CLR(fd, &fdset);
                                        close(fd);
                                        return -1;
                                    }
                                }
                                //监听到连接
                                /*ret = read_port(fd, buf, 60);
                                if(strstr(buf, "END") != NULL)
                                {
                                    //证明扫描到末尾了,但是还没有找到
                                    return -1;
                                }
                                else
                                {
                                    //没有扫到末尾
                                    string temp_scan(buf, buf + ret);
                                    cout << temp_scan << endl;
                                    if(strstr(buf, "g2020") != NULL)
                                    {
                                        //扫到g2020这个蓝牙了
                                        string mac(buf, buf + 11);
                                        device_mac = mac;
                                        return fd;
                                    } 
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    FD_CLR(fd, &fdset);
    close(fd);
    return false;
}

*/
void* RS485_Monitor(void *arg){   //用于无人船时，将5号给注释掉
    cout<<"Enabling Serial Port Listening!"<<endl;
    char buf[1024];
    struct monitor485 *monitor485 = (struct monitor485 *)arg;
                                                //开启线程注册(后续可以加入)
    int fdRs485One = open_port(5);              //打开串口
    cout<<"fdRs485One:"<<fdRs485One<<endl;
    int fdRs485Two = open_port(6); 
    cout<<"fdRs485Two:"<<fdRs485Two<<endl;
    int fdRs232 = open_port(7);
    cout<<"fdRs232:"<<fdRs232<<endl;
    //int maxNum = fdRs485One > fdRs485Two ? fdRs485One : fdRs485Two;
    int maxNum = fdRs232;            
    if(fdRs485One == -1 || fdRs485Two == -1 || fdRs232 == -1){
        return false;
    }else{
        set_opt(fdRs485One, 9600, 8, 'n', 1, 1);     //并对其波特率和流控制进行设置
        set_opt(fdRs485Two, 9600, 8, 'n', 1, 1);
        set_opt(fdRs232, 9600, 8, 'n', 1, 1);
    }
    struct timeval timeout;
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fdRs485One,&fdset);
    FD_SET(fdRs485Two,&fdset);
    FD_SET(fdRs232,&fdset);
    cout<<"File Descriptor-1："<<fdRs485One<<endl;
    cout<<"File Descriptor-2："<<fdRs485Two<<endl;
    cout<<"File Descriptor-3："<<fdRs232<<endl;
    int ret;
    while(1){
        FD_ZERO(&fdset);
        FD_SET(fdRs485One, &fdset);
        FD_SET(fdRs485Two, &fdset);
        FD_SET(fdRs232, &fdset);  
        // 检测设备启动
        // 监听事件 1秒钟 超时跳过
        ret = select(maxNum + 1, &fdset, NULL, NULL, NULL);
        if(ret>0){
            cout<<"Serial Port Has Data"<<endl;
            if (FD_ISSET(fdRs485One, &fdset)){
                ret = read(fdRs485One, buf, 1024);
                if(ret>0){
                    string str = buf; 
                    str = "##" + str;
                    string strr = packet06(str, RS485_A);     //对读取到数据进行06封包处理
                    monitor485->que->push(strr);     //将数据存储到缓存队列里面
                    cout<<"The Port RS485 Data is："<<buf<<endl;
                } 
                memset(buf,0,strlen(buf));          //清空，防止消息重复 
            }
            if (FD_ISSET(fdRs485Two, &fdset)){
                ret = read(fdRs485Two, buf, 1024);
                if(ret>0){
                    string str = buf;
                    str = "##" + str;
                    string strr = packet06(str, RS485_C);     //对读取到数据进行06封包处理
                    monitor485->que->push(strr);     //将数据存储到缓存队列里面
                    cout<<"The Port RS485 Data is："<<buf<<endl;
                } 
                memset(buf,0,strlen(buf));          //清空，防止消息重复 
            }
            if (FD_ISSET(fdRs232, &fdset)){
                ret = read(fdRs232, buf, 1024);
                if(ret>0){
                    string str = buf;
                    str = "##" + str;
                    string strr = packet06(str, RS232);     //对读取到数据进行06封包处理
                    monitor485->que->push(strr);     //将数据存储到缓存队列里面
                    cout<<"The Port RS232 Data is："<<buf<<endl;
                } 
                memset(buf,0,strlen(buf));          //清空，防止消息重复 
            }
        }
    }
}