#include "registManage.h"

/* 设备注册的线程函数
* 1.初始化，对初始化的设备发送01包请求注册
* 2.监听并接受ISR时间戳 17包
* 3.发送03包进行注册确认
* 4.等待时间戳下发 20 包
* 
*/

void* device_regist(void* arg)
{
    cout << "device_regist start!!!" << endl; 
    struct timeval tv1; //超时时间
    struct timeval tv2;
    uint8_t RX_buf[RX_SIZE];    //接收数组,最大58字节
    int myCount = 0;  //计数器
    // string accessdev_gps = gps_lat + "," + gps_lon;
//regist_message = regist_message + "$" + "01" + "1" + id + net_id + "00" + "0026" + mac + communicate_status +"010"+ gps_lon+gps_lat+cpu_occupy+"@";  //注册请求消息
    int ret;
    int select_num = 1;
    int regist_num = 1;           //一个通信设备注册次数的计数器
    fd_set fdset;
    FD_ZERO(&fdset);
    struct deviceRegist* dev = (struct deviceRegist*)arg; //参数强转
    CSoftwareWdt* g_CsoftwareWdt = dev->g_CsoftwareWdt;
    vector<vector<int>>* vec = dev->vec;
    int size = (*vec).size() - 1;   //获取初始化成功的通信设备组大小
    const char* const threadname = "device_regist";      //设定此线程的名字
    int wdt_id = -1;
    const int softdogTimeout = 60;
    wdt_id = g_CsoftwareWdt->RequestSoftwareWdtID(threadname, softdogTimeout);//线程注册
    cout << " device_regist 1" << endl;
    g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id); //此线程心跳操作
    cout << " device_regist 1" << endl;
select_again:  //标记
    if (regist_num == 5) //一个通信设备注册5次失败，换下一个初始化成功的通信设备开始注册
    {
        size--; //减少设备组索引 size
        regist_num = 1; //重置注册次数计数器
        cout << "下一个通信设备开始注册" << endl;
        sleep(5);
    }
    if (size < 0)  //一轮已经注册完毕，没有注册成功的设备,此时可能isr正在重启或则休眠避免重复重启，休眠5分钟//
    {
        //sleep(300);
        size = (*vec).size() - 1;//重置 size
        stop(0);
        cout << "一轮已经注册完毕, 退出注册线程" << endl;
        return (void*)-1; //通常用来表示线程执行过程中出现错误或者失败  它把 -1 从 int 类型转换为 void* 类型
        //system("reboot -f");

    }
   
    while (1) //主监听及数据处理循环
    {
        FD_ZERO(&fdset); //清空文件描述符集合 fdset
        FD_SET((*vec)[size][1], &fdset);	 //将初始化成功的通信设备描述符加入监听数组	 (*vec)[size][1]获取第 size 个通信设备的文件描述符 fd
        //把指定通信设备的文件描述符添加到 fdset 集合中，这样在后续调用 select 函数时，就能够对该通信设备的文件描述符进行监控
        tv1.tv_sec = 15;// 超时时间为 15 秒
        tv1.tv_usec = 0; // 微秒
        myCount++;//
        if (myCount == 1000) return (void*)-1;  //容错机制
        cout << " device_regist 8" << endl;  //这里有问题
        g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id); //此线程心跳操作
        cout << " device_regist 8" << endl;
        ret = select((*vec)[size][1] + 1, &fdset, NULL, NULL, &tv1);  //监听15s 监控 读 事件
        cout << " device_regist 8 ----- 8" << endl;
        if (ret > 0)							//监听到事件发生
        {
            if (FD_ISSET((*vec)[size][1], &fdset)) //设备文件描述符在集合中，且事件已发生
            {
                memset(RX_buf, 0, sizeof(RX_buf));//初始化为0
                ret = read((*vec)[size][1], RX_buf, RX_SIZE);
                if (ret < 58 && RX_buf[0] != '$' && RX_buf[ret - 1] == '@') //该包为最后一个数据包
                {
                    cout << " device_regist 2" << endl;
                    g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
                    cout << " device_regist 2" << endl;
                    goto sendreg_entrance;  //向对应的通信设备描述符写入注册请求信息
                }
                else
                {
                    if (RX_buf[0] == '$' && RX_buf[ret - 1] == '@' && select_num == 5)
                    {
                        cout << " device_regist 3" << endl;
                        g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
                        cout << " device_regist 3" << endl;
                        goto sendreg_entrance;
                    }
                    else
                    {
                        select_num++;
                    }
                }
            }
            // cout<<"喂狗操作"<<endl;
        }
        else if (ret == 0)	//此时信道空闲，可以直接发送注册信息
        {
            cout << " device_regist 4" << endl;
            g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
            cout << " device_regist 4" << endl;
            goto sendreg_entrance;
        }
        else {  //出错直接返回
            return (void*)-1;
        }
    }//while结束
sendreg_entrance:  			//向对应的通信设备描述符写入注册请求信息
    sleep(monitor_time / 4);   //暂停四分之一个监听周期
    string regist_message = "";
    regist_message = regist_message + "$" + "01" + to_string((*vec)[size][0]) + id + net_id + "00" + "0026" + mac + communicate_status + "010" + gps_lon + gps_lat + cpu_occupy + "@";  //注册请求消息
    ret = write((*vec)[size][1], regist_message.c_str(), strlen(regist_message.c_str()));    //写入注册请求命令 到对应的通信设备描述符中
    if (ret <= 0) //写入失败  																			//注册命令写入失败
    {
        cout << "device_regist: write error:" << strerror(errno) << endl;
        select_num = 1;
        regist_num++; //注册次数
        goto select_again;
    }
    else		//注册命令写入成功
    {
        cout << "ret:" << ret << endl;
        cout << "Regist: " << regist_message << endl;
        int i = 5;
        while (i--)     //开始监听是否收到ISR的17消息，尝试 5 次
        {
            FD_ZERO(&fdset);
            FD_SET((*vec)[size][1], &fdset);
            tv2.tv_sec = monitor_time;
            tv2.tv_usec = 0;
            cout << " device_regist 5" << endl;
            g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
            cout << " device_regist 5" << endl;
            ret = select((*vec)[size][1] + 1, &fdset, NULL, NULL, &tv2);
            cout << "ret >> " << ret << endl;
            if (ret > 0)
            {
                cout << " device_regist 6" << endl;
                g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id);
                cout << " device_regist 6" << endl;
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read((*vec)[size][1], RX_buf, RX_SIZE);
#ifdef DEBUG 
                cout << "Regist recv：" << RX_buf << endl;
#endif            		
                //对接收到的指令消息进行分解
                string regist_recv_message(RX_buf, RX_buf + ret);
                string recv_protocal = regist_recv_message.substr(1, 2); //从第二个字符开始提取两个字符
                if (recv_protocal == REQ_SEND_INFO) 						//如果是参数下发消息
                {
                    string recv_mac = regist_recv_message.substr(16, 16);   //获取下发的mac地址，设备唯一码
                    if (recv_mac == mac)
                    {
                        Isr_mac = regist_recv_message.substr(32, 16);          //获取网关设备ISR的MAC地址
                        id = regist_recv_message.substr(48, 2);             //获取下发的id
                        cout << "网关下发的ID：" << id << endl;
                        net_id = regist_recv_message.substr(50, 4);         //获取所在网络的id
                        break;
                    }
                    else 													//接收到的是参数下发的消息，但是mac地址不对应，则重新接收
                    {
                        continue;
                    }
                }
                else														//接收到的消息不是参数下发的，则重新接收
                {
                    continue;
                }
            }
        }
        /*设备注册成功后，进行注册确认消息回传以及监听时间戳下发*/
        if (id != "FF")  //设备ID更新成功
        {
            int listen_num = 8; //监听次数
            //设备注册成功，回传 *注册确认* 消息  并监听时间戳下发
            string confirm_message = ""; //确认消息
            confirm_message = confirm_message + "$" + "02" + to_string((*vec)[size][0]) + id + net_id + "00" + "0012" + mac + id + "@";
            //cout<<" 02:"
            ret = write((*vec)[size][1], confirm_message.c_str(), strlen(confirm_message.c_str()));
            if (ret <= 0)
            {
                printf("write confirm_message failed!");
            }
            cout << "confirm_message:" << confirm_message << endl;
            cout << "等待下发时间戳" << endl;
            while (listen_num)  //加一个判断，监听次数
            {
                cout << " device_regist 7" << endl;
                g_CsoftwareWdt->KeepSoftwareWdtAlive(wdt_id); //进行软件看门狗的心跳操作
                cout << " device_regist 7" << endl;
                FD_ZERO(&fdset);//清空文件描述符集合
                FD_SET((*vec)[size][1], &fdset);
                ret = select((*vec)[size][1] + 1, &fdset, NULL, NULL, NULL); //继续开始监听时间戳消息，在此堵塞式监听  (可能的有问题部分，可能会一直阻塞在这里)
                listen_num--;
                cout << "时间戳监听计数器：" << listen_num << endl;
                if (ret > 0)
                {
                    memset(RX_buf, 0, sizeof(RX_buf));//清空接收缓冲区
                    ret = read((*vec)[size][1], RX_buf, RX_SIZE); //读取时间戳数据到 RX_buf
                    if (ret == 0) {  //文件末尾
                        cout << "ISR QUIT" << endl;
                        continue;
                    }
                    string time_recvmessage(RX_buf, RX_buf + ret);
                    cout << time_recvmessage << endl;
                    string recv_protocal = time_recvmessage.substr(1, 2);
                    cout << recv_protocal << endl;
                    string recv_mac = time_recvmessage.substr(16, 16);
                    cout << recv_mac << endl;
                    if (recv_protocal == TIME_SEND && recv_mac == mac)
                    {
                        current_time = time_recvmessage.substr(32, 17);   //获取网关下发参数时间戳 2016 08 01 08 58 57 223
                        string year_time_info = current_time.substr(0, 4); //年
                        string month_time_info = current_time.substr(4, 2);//月
                        string day_time_info = current_time.substr(6, 2);//日
                        string hour_time_info = current_time.substr(8, 2); //时
                        string minute_time_info = current_time.substr(10, 2);//分
                        string sec_time_info = current_time.substr(12, 2);//秒
                        string msec_time_info = current_time.substr(14, 3);//毫秒
                        char time[50] = "";
                        sprintf(time, "date %s-%s-%s", year_time_info.c_str(), month_time_info.c_str(), day_time_info.c_str());
                        cout << "current year :" << time << endl; //年月日
                        system(time);
                        memset(time, 0, sizeof(time));
                        sprintf(time, "date %s:%s:%s", hour_time_info.c_str(), minute_time_info.c_str(), sec_time_info.c_str());
                        cout << "current hour :" << time << endl;//时分秒
                        system(time);
                        if (CM->setEnable((*vec)[size][0]) == true)   //将其设置为注册成功使能状态
                        {
                            cout << "ID号为：" << (*vec)[size][0] << "使能" << endl;
                            cout << "SAP注册成功" << endl;
                            g_CsoftwareWdt->ReleaseSoftwareWdtID(threadname, wdt_id); //解除对此线程的监控
                            // pthread_exit(NULL);                                      //此线程退出
                            return (void*)0;
                        }
                        break;
                        //return true;
                    }
                }
            }
            if (listen_num == 0) {  //未接收到有效的时间戳消息
                stop(0);
                return (void*)-1; //此次未注册成功，重新开始注册 
            }
        }
        else
        {
            select_num = 1;
            regist_num++;
            goto select_again;
        }
    }
}

/*
//设备注册线程，对其进行检测，如果异常直接关机重启
void device_regist(void* arg)
{
    cout << "device_regist start!!!" << endl;
    struct deviceRegist *dev = (struct deviceRegist *)arg;
    const char* const threadname ="device_regist";
    int wdt_id = -1;
    const softdogTimeout = 60;
    wdt_id=dev->g_CsoftwareWdt->RequestSoftwareWdtID(threadname,softdogTimeout);//线程注册
    struct timeval tv1;
    struct timeval tv2;
    uint8_t RX_buf[RX_SIZE];                    //接收字符串
                   // string accessdev_gps = gps_lat + "," + gps_lon;
    //regist_message = regist_message + "$" + "01" + "1" + id + net_id + "00" + "0026" + mac + communicate_status +"010"+ gps_lon+gps_lat+cpu_occupy+"@";  //注册请求消息
    int ret;
    int select_num = 1;
    int regist_num=1;
    fd_set fdset;
    FD_ZERO(&fdset);
    int size=vec.size()-1;
select_again:
    if(regist_num==5)                           //一个通信设备注册5次失败，换下一个通信设备开始注册
    {
        size--;
        regist_num=1;
        cout<<"下一个通信设备开始注册"<<endl;
        sleep(4);
    }
    if(size<0)
    {
        size=vec.size()-1;
    }
    while(1)
    {
        FD_ZERO(&fdset);
        FD_SET(vec[size][1], &fdset);           //将初始化成功的通信设备描述符加入监听数组
        tv1.tv_sec = 15;
        tv1.tv_usec = 0;
        ret = select(vec[size][1] + 1, &fdset, NULL, NULL, &tv1);  //监听15s
        if (ret > 0)                            //监听到事件发生
        {
            if (FD_ISSET(vec[size][1], &fdset))//测试监听的设备描述符是否可读
            {
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read(vec[size][1], RX_buf, RX_SIZE);
                if (ret < 58 && RX_buf[0] != '$' && RX_buf[ret - 1] == '@') //该包为最后一个数据包
                {

                    goto sendreg_entrance;
                }
                else
                {
                    if(RX_buf[0] == '$' && RX_buf[ret - 1] == '@' && select_num == 5)
                    {
                        goto sendreg_entrance;
                    }
                    else
                    {
                        select_num++;
                    }
                }
            }
        }else if(ret==0)    //此时信道空闲，可以直接发送注册信息
        {
            goto sendreg_entrance;
        }
    }
sendreg_entrance:           //向对应的通信设备描述符写入注册请求信息
    sleep(monitor_time / 4);   //暂停四分之一个监听周期
    string regist_message = "";
    regist_message = regist_message + "$" + "01" + to_string(vec[size][0]) + id + net_id + "00" + "0026" + mac + communicate_status +"010"+ gps_lon+gps_lat+cpu_occupy+"@";  //注册请求消息
    ret = write(vec[size][1], regist_message.c_str(), strlen(regist_message.c_str()));      //写入注册请求命令
    if(ret<=0)                                                                              //注册命令写入失败
    {
        cout << "device_regist: write error:" << strerror(errno) << endl;
        select_num = 1;
        regist_num++;
        goto select_again;
    }else                                                                                   //注册命令写入成功
    {
        cout << "ret:" << ret << endl;
        cout << "Regist: " << regist_message << endl;
        int i = 5;
        while(i--)         //开始监听是否收到ISR的07命令
        {
            FD_ZERO(&fdset);
            FD_SET(vec[size][1], &fdset);
            tv2.tv_sec = monitor_time;
            tv2.tv_usec = 0;
            ret = select(vec[size][1] + 1, &fdset, NULL, NULL, &tv2);
            cout << "ret >> " << ret << endl;
            if(ret>0)
            {
                memset(RX_buf, 0, sizeof(RX_buf));
                ret = read(vec[size][1], RX_buf, RX_SIZE);
#ifdef DEBUG
                cout << "Regist recv：" << RX_buf << endl;
#endif
                //对接收到的指令消息进行分解
                string regist_recv_message(RX_buf, RX_buf + ret);
                string recv_protocal = regist_recv_message.substr(1, 2);
                if (recv_protocal == REQ_SEND_INFO)                         //如果是参数下发消息
                {
                    string recv_mac = regist_recv_message.substr(16, 16);   //获取下发的mac地址，设备唯一码
                    if (recv_mac == mac)
                    {
                        Isr_mac=regist_recv_message.substr(32,16);          //获取网关设备ISR的MAC地址
                        id = regist_recv_message.substr(48, 2);             //获取下发的id
                        cout<<"网关下发的ID："<<id<<endl;
                        net_id = regist_recv_message.substr(50, 4);         //获取所在网络的id
                        break;
                    }else                                                   //接收到的是参数下发的消息，但是mac地址不对应，则重新接收
                    {
                        continue;
                    }
                }else                                                       //接收到的消息不是参数下发的，则重新接收
                {
                    continue;
                }
            }
        }
        if (id != "FF")  //设备ID更新成功
        {
            int listen_num=5;
            //设备注册成功，回传注册确认，并退出
            string confirm_message = "";
            confirm_message = confirm_message + "$" + "02" + to_string(vec[size][0]) + id + net_id + "00" + "0012" + mac + id + "@";
            ret = write(vec[size][1], confirm_message.c_str(), strlen(confirm_message.c_str()));
            if (ret <= 0)
            {
                printf("write confirm_message failed!");
            }
            cout << "confirm_message:" << confirm_message << endl;
            cout << "等待下发时间戳"<<endl;
            while(listen_num)
            {
                FD_ZERO(&fdset);
                FD_SET(vec[size][1], &fdset);
                ret = select(vec[size][1] + 1, &fdset, NULL, NULL,NULL); //继续开始监听时间戳消息
                listen_num--;
                cout<<listen_num<<endl;
                if(ret>0)
                {
                    memset(RX_buf, 0, sizeof(RX_buf));
                    ret = read(vec[size][1], RX_buf, RX_SIZE);
                    string time_recvmessage(RX_buf,RX_buf+ret);
                    cout<<time_recvmessage<<endl;
                    string recv_protocal =time_recvmessage.substr(1, 2);
                    cout<<recv_protocal<<endl;
                    string recv_mac = time_recvmessage.substr(16, 16);
                    cout<<recv_mac<<endl;
                    if(recv_protocal==TIME_SEND && recv_mac==mac )
                    {
                        current_time = time_recvmessage.substr(32, 17);   //获取网关下发参数时间戳 2016 08 01 08 58 57 223
                        string year_time_info = current_time.substr(0, 4);
                        string month_time_info = current_time.substr(4, 2);
                        string day_time_info = current_time.substr(6, 2);
                        string hour_time_info = current_time.substr(8, 2);
                        string minute_time_info = current_time.substr(10, 2);
                        string sec_time_info = current_time.substr(12, 2);
                        string msec_time_info = current_time.substr(14, 3);
                        char time[50] = "";
                        sprintf(time, "date %s-%s-%s", year_time_info.c_str(), month_time_info.c_str(), day_time_info.c_str());
                        cout << "current year :" << time << endl;
                        system(time);
                        memset(time, 0, sizeof(time));
                        sprintf(time, "date %s:%s:%s", hour_time_info.c_str(), minute_time_info.c_str(), sec_time_info.c_str());
                        cout << "current hour :" << time << endl;
                        system(time);
                        if(CM->setEnable(vec[size][0])==true)   //将其设置为注册成功使能状态
                        {
                            cout<<"ID号为："<<vec[size][0]<<"使能"<<endl;
                            cout<<"SAP注册成功"<<endl;
                        }
                        break;
                        //return true;
                    }
                }
            }
        }else
        {
            select_num = 1;
            regist_num++;
            goto select_again;
        }
    }
}
*/
