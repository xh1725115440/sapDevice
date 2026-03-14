#include "protocol_process.h"
#include "access_device.h"
#include <vector>
#include <time.h>
extern string Isr_mac, id, net_id, REQ_SEND_INFO, mac, REQ_DATA, gps_lat, gps_lon, current_time ;
extern char communicate_status[];
extern communicaManage* CM;
extern int fd_lora;
#define REQ_REGISTER            "01"                    //注册请求
#define REQ_REG_CONFIRM         "02"                    //注册确认
#define REQ_DATA                 "03"                    //数据请求
#define REQ_TXD_CONFIRM         "04"                    //数据上传确认
#define REQ_PARAM_CONFIRM       "05"                    //传感器参数变更执行确认
#define REQ_PARAM_SEND          "06"                    //传感器参数变更命令下发
#define REQ_ID_CONFIRM          "07"                    //接入设备ID变更执行确认
#define REQ_ID_SEND             "08"                    //接入设备ID变更命令下发
#define REQ_MONI_RET_CONFIRM    "09"                    //监测终端重置执行确认
#define REQ_MONI_RET            "10"                    //监测终端重置命令下发
#define REQ_ACC_RET_CONFIRM     "11"                    //接入设备重启执行确认
#define REQ_ACC_RET             "12"                    //接入设备重启命令下发
#define REQ_KEY_CONFIRM         "13"                    //更新通信秘钥执行确认
#define REQ_KEY_SEND            "14"                    //更新通信秘钥命令下发
#define REQ_ASK_KEY             "15"                    //请求通信秘钥
#define REQ_SEND_KEY            "16"                    //下发通信秘钥
#define REQ_SEND_INFO           "17"                    //网关参数下发
#define REQ_ACC_CLOSE           "18"                    //接入设备退网
#define REQ_GATEWAY_CLOSE       "19"                    //网关设备退网
#define TIME_SEND               "20"                    //时间戳下发
#define REQ_ACC_HEART           "EF"                    //接入设备心跳包
#define REQ_GATE_HEART          "FF"                    //网关设备心跳包
#define TESTPATHNAME            "/home/root/myfile.txt"
#define STOREFILEPATHNAME       "/home/root/storefile.txt"
/*
作为进程监控的测试使用
*/
void mytest(string buffer){
    time_t t = time(NULL);       //返回系统的当前时间
    char *fileNameT = ctime(&t);   //生成临时日期字符串
    string timeT = fileNameT + buffer;
    const char *buff = timeT.c_str();
    int fd = open(TESTPATHNAME, O_APPEND | O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG |S_IRWXO);
    if(fd < 0) perror("open in mytest : ");
    int numwrite = write(fd, buff, strlen(buff));
    if(numwrite < 0) perror("write in mytest:");
    close(fd);
}

/**
*   将int型转换成十六进制字符串
*   @param num 需要转换的数据
*   @return
*
*/

void stop(int signum)
{
#ifdef DEBUG 
    printf("Stop the program!\n");
#endif 
   // string close_DISCONNECT = "AT+DISCON=1\r\n";
    //close(fd_wifi);
    //close(fd_lora);
   // close(fd_bt);
   // close(fd_lan);
    //close(fdL_lan);
    //system("/home/root/jd_open.sh");
    //sleep(1);
    //write_port(fd_bt, close_DISCONNECT.c_str(), strlen(close_DISCONNECT.c_str()));
    //system("/home/root/jd_close.sh");
    //sleep(2);
    system("echo 0 > /sys/class/leds/red/brightness");
    system("echo 0 > /sys/class/leds/yellow/brightness");
    system("echo 0 > /sys/class/leds/green/brightness");
    system("/home/root/power_12v.sh off");
    system("hwclock -w");  

    system("killall wpa_supplicant");            //关闭所有wpa_supplicant进程
    sleep(1);
    system("killall udhcpc");                    //关闭所有udhcpc进程
    return ;
}
string switch10_16(int num)  //规定余数输出格式的函数，0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
{
    switch(num){
        case 0:return"0";
                    break;
        case 1:return"1";
                    break;
        case 2:return"2";
                    break;
        case 3:return"3";
                   break;
        case 4:return"4";
                   break;
        case 5:return"5";
                   break;
        case 6:return"6";
                   break;
        case 7:return"7";
                    break;
        case 8:return"8";
                    break;
        case 9:return"9";
                    break;
        case 10:return"A";
                      break;
        case 11:return"B";
                      break;
        case 12:return"C";
                      break;
        case 13:return"D";
                      break;
        case 14:return"E";
                      break;
        case 15:return"F";
                      break;
    }
}
/**
*   将长度转化为16进制，并且按照string类型进行返回
*   @param num 需要转换的数据
*   @param str 必须为空字符串
*   @return
*
*/
int change(long long int num, string& str)  //转换函数，把十进制转换成十六进制
{
    if(num<16)    //如果要转换的数小于16，直接找到格式函数，把转换后的数加到字符串后面
    {
        str+=switch10_16(num);
        return 0;
    }
    if(num>16) //如果要转换的数大于16
    {
        change(num/16,str);   //一直除16，相当于辗转相除法，不懂可以参考十进制算十六进制的过程
        str+=switch10_16(num%16); //把余数加到字符串里
    }
}
//气象仪数值处理函数
//功能：将采集到的数据转换为对应的格式 eg：1234-->123.4
//num:采集到的数值
/*char* int2pString(int num)
{
    int a=num%10;
    int b=num/10;
    char str[]={};
    sprintf(str,"%d.%d",b,a);
    return str;
}*/
float int2pString(int num)
{
    int a=num%10;
    int b=num/10;
    string str="";
    sprintf((char*)str.c_str(),"%d.%d",b,a);
    float f=atof(str.c_str());
    return f;
}
//注册请求处理函数
//@param
//buf   接收到的字符串
//num   现有的接入设备的编号
//net   网络id
//
//return 返回需要下发的字符串
string dev_register(string buf, string *num, string net)
{
    string mac = buf.substr(14, 16);
    string temp_num = *num;
    char h = temp_num[0], l = temp_num[1];
    if(temp_num[1] == 'F')
    {
        l = '0';
        h = h + 1;
        *num = h + l;
    }
    string out = "";
    out =out + "$" + "19" + "1" + "00" + net + "00" + "12" + mac + (*num) + "@"; 
    return out;
}

//注册确认处理函数
//功能：对注册确认消息 包进行处理
//@param
//buf   注册确认数据包
//que   当前接入设备队列
bool dev_reg_confirm(string buf, vector<access_device> *que)
{
    string mac = buf.substr();
}

//17协议处理函数
//@param
//buff  收到的17消息
//确认是下发给本机的17消息返回17且向ISR发送02消息，否则返回-1
int protoc_17(string buff)
{   
    int ret;
    string confirm_message="";
    string recv_protocal = buff.substr(1, 2);
    if (recv_protocal == REQ_SEND_INFO)
    {

        string recv_mac = buff.substr(16, 16);
        if (recv_mac == mac)                                    //确认是该接入设备发起的注册请求
        {
            Isr_mac=buff.substr(32,16);          //获取网关设备ISR的MAC地址
            id = buff.substr(48, 2);             //获取下发的id
            net_id = buff.substr(50, 4);
            confirm_message = confirm_message + "$" + "02" + "1" + id + net_id + "00" + "0012" + mac + id + "@"; 
            ret = write(fd_lora, confirm_message.c_str(),strlen(confirm_message.c_str()));
            if (ret <= 0)
            {
                printf("write confirm_message failed!");
                return -1;
            }else{
                cout << "confirm_message:" << confirm_message << endl;
                return 17;
            }

        }else 
        {

            return -1;
        } 
    }else
    {
        return -1;
    }
}
//03协议处理函数
//@param
//buff  收到的03消息
//传入传出参数，保存03下发的数据发送方式
//确认是下发给本机的03消息返回3，否则返回-1
int protoc_03(const string buff,string *communicate_method)
{
    string recv_protocal = buff.substr(1, 2);
    if (recv_protocal == REQ_DATA)
    {
        string recv_mac = buff.substr(22, 16);
        if (recv_mac == mac)
        {
            *communicate_method = buff.substr(38, 4);
        }else
        {
            return -1;
        }

    }else
    {
        return -1;
    }
    return 3;
}
/*  <summary>
     06封包函数处理函数
    </summary>
    <param name="strT">需要进行封包处理的数据</param>
    <param name="portInfo">SAP端口号</param>
    <returns>封装好的06包</returns>
 */
 
string packet06(string strT, string portInfo)
{
    char packet[1024]={0};
    string cpu_mem = get_cpuOccupy(); //获取CPU内存使用率
    //string accessdev_gps = gps_lat + gps_lon; //获取GPS信息
    string accessdev_gps = "N2932E10636";
    int length= (strlen(strT.c_str())+54);  //计算封包后的包长度
    string str = ""; 
    change(length, str);
    int num=str.length();
    if(num==4)
    {

    }else if(num==3)
    {
        str= "0" +str;
    }else if(num==2)
    {
        str="00" +str;
    }else if(num==1)
    {
        str="000"+str;
    }
  
    string communicateType = to_string(CM->getSuccessId()); //当前通信模块类型
    memset(packet,0,strlen(packet));
    sprintf((char*)packet,"$06%s%s%s00%s%s%s%s%s%s%s%s@",communicateType.c_str(),id.c_str(),net_id.c_str(),\
        str.c_str(),Isr_mac.c_str(),mac.c_str(),portInfo.c_str(),accessdev_gps.c_str(),cpu_mem.c_str(),\
        communicate_status,strT.c_str());  //按照指定格式将一些数据输入到字符串
    //sprintf((char*)packet,"$06%s%s%s00%s%s%s010%s%s%s%s@",communicateType.c_str(),id.c_str(),net_id.c_str(),\
        str.c_str(),Isr_mac.c_str(),mac.c_str(),accessdev_gps.c_str(),cpu_mem.c_str(),\
        communicate_status,strT.c_str());
    string packets06 = packet;
    return packets06 ;
}

void packet20(string& strPacket){
    //string time_recvmessage(RX_buf,RX_buf+ret);
    cout<<"Start deal 20 time packet"<<endl;
    //string recv_protocal =strPacket.substr(1, 2);
    //cout<<recv_protocal<<endl;
    string recv_mac = strPacket.substr(16, 16);
    //cout<<recv_mac<<endl;
    if(recv_mac==mac )
    {
        current_time = strPacket.substr(32, 17);   //获取网关下发参数时间戳 2016 08 01 08 58 57 223
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
        cout<<"__________更新时间成功_________"<<endl;
    }
}

int dataTofile(queue<string>& buffer){  //向文件写入还未发出的数据
    int fd = open(STOREFILEPATHNAME, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU | S_IRWXG |S_IRWXO);
    if(fd < 0) perror("in dataTofile open: ");
    int count = 0;
    while(!buffer.empty()){
        const char* buff = buffer.front().c_str();
        int num = write(fd, buff, strlen(buff));
        const char* b = "\n";
        write(fd, b, strlen(b));
        if(num <= 0) perror("in dataTofile write: ");
        buffer.pop();
        count ++;
    }  
    return count; 
}

int readFromFile(queue<string>& recvQueue){ //返回当前读取到多少条数据
    FILE *fs = fopen(STOREFILEPATHNAME, "r");  //打开要读取的文本文件，以只读的方式打开
    if (fs == NULL) {
        perror("fopen:");
        return -1;
    }
    char* buffer = NULL;
    size_t num = 0;
    int count = 0;
    while(getline(&buffer, &num, fs) != -1){

        buffer[num - 1] = '\0';//将换行符替换成'\0' 错误应该将换行符给去掉

        buffer[1] = '2'; //将06包转成21重发包
        buffer[2] = '1';
        const char* buff = buffer;
        int length = strlen(buff);
        string ss = buffer;
        cout << "未截取的字符串长度:" <<ss.length()<<endl;
        string s = ss.substr(0, length - 1);
        cout << "截取后的字符串长度:" <<s.length()<<endl;
        recvQueue.push(s);

        count ++;
    }
    if(buffer){
        free(buffer);
    }
    int fd = open(STOREFILEPATHNAME, O_TRUNC);
    if(fd < 0) perror("in dataTofile open: ");
    return count;
}

