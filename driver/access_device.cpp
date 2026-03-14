#include "access_device.h"

#define DEBUG             //用于调试的DEBUG
//获取设备的信息函数
string access_device::get_id(){return id;}
string access_device::get_netid(){return net_id;}
string access_device::get_mac(){return mac;}
bool access_device::get_op(){return op;}
string access_device::get_mess(){return message;}
char access_device::get_st(){return mess_st;}
int access_device::get_m_num(){return mess_num;}
char access_device::get_en(){return mess_en;}
int access_device::get_fre(){return freque;}
int access_device::get_heart(){return hearbeat;}
//修改设备的信息函数
void access_device::change_op(bool &t_op){op = t_op;}
void access_device::change_fre(int &t_fre){freque = t_fre;}
void access_device::change_id(string &t_id){id = t_id;}
void access_device::change_heart(int t_heart){hearbeat = t_heart;}
//接入设备参数重置
void access_device::clear_info(){
    op = false;
    message = "";
    mess_num = -1;
    hearbeat = 0;
}
//创建接入设备类函数
//@param 
//buf   接收到的字符串
//num   现有的接入设备的编号
//net   网络id
//初始化接入设备，设置id,mac,op等
void access_device::create_device(string &buf, string &num, string &net, int &fre)
{
    string t_mac = buf.substr(14, 16);
    id = num;
    net_id = net;
    mac = t_mac;
    freque = fre;
    mess_num = -1;
    op = false;     //表明设备现在只是注册，还未确认，所以还未开机
    hearbeat = 0;
}

//注册请求处理函数
//@param
//id        接收到的id
//mac       接入设备的mac
//net_id    网络id
//
//return 返回需要下发的字符串
string access_device::regist_device()
{
    string out = "";
    //下发频率转换为16进制
    stringstream ss;
    string s_fre = "";
    char h = '0', l = '0';
    h = h + (freque / 16);
    l = l + (freque - (freque / 16) * 16);
    s_fre = s_fre + h + l;
    out =out + "$" + "17" + "1" + "00" + net_id + "00" + "14" + mac + id + s_fre + "@"; 
    return out;
}


//注册确认处理函数
//功能：用于设备的注册确认，成功注册的修改开机标识符
//@param
//t_id      确认信息包上传的id
//t_netid   确认信息包上传的netid
//t_mac     确认信息包上传的设备的mac
//
//return    空
void access_device::confirm_device(string &t_id, string &t_netid, string &t_mac)
{
    if(t_id == id && t_netid == net_id && t_mac == mac)
    {
#ifdef DEBUG 
        cout << "Confirm_device information is suitable\n";
#endif
        op = true;
    }
    else{
        cout << "Confirm_device information is wrong\n";
    }
}

//数据上传函数
//功能：处理接收到的接入设备上传的数据。
//      完整的数据则返回数据包，开始上传
//      不完整则记录相关信息，并返回空
//@param
//rx_mess 接收到的上传的数据包
string access_device::send_device(string &rx_mess)
{
    //提取出数据字段
    int pos_mess_st = 14;
    int pos_mess_en = rx_mess.find('@');
    string recv_message = rx_mess.substr(14, pos_mess_en - pos_mess_st);        //提取出上传的数据包字符串
    int recv_num = (rx_mess[10] - '0') * 16 + (rx_mess[11] - '0');      //获取剩余的数据包的数量 
    string out = "";

    //表明此时是单个数据包，且未分包
    if(-1 == mess_num && true == op && recv_num == 0)
    {
        out = recv_message;
    }
    //此时不是单个数据包，分包了的
    //判断当前是否是开始数据包
    if(-1 == mess_num && true == op && recv_num != 0)
    {
        //-1表示之前并未接收到数据包
        //此时是第一个数据包,且设备在网络中已被激活
        message = recv_message;
        mess_st = '$';
        mess_num = recv_num;
        mess_en = '@';
    }
    else if(-1 != mess_num && true == op)
    {
        //此时不是第一个数据包，且设备在网络中已被激活
        //先更新数据包
        //判断是否是最后一个数据包
        if(0 == recv_num)
        {
            //表明此时是最后一个数据包，则需要返回上传的数据包，同时将接入设备的相关信息设置为空
            message += recv_message;
            out = message;
            message = "";
            mess_num = -1;
        }
        else if(0 != recv_num /*&& 1 == mess_num - recv_num*/)
        {
            //该数据包后还有recv_num个数据包，且判断当前数据包是否是上一个数据包后的包
            //确认后，将mess_num修改
            message += recv_message;
            mess_num = recv_num;
        }
    }
    return out;
}

//网关设备心跳包
//用于网关设备接收到接入设备发送的心跳包后，回传一个心跳包给接入设备，告知接入设备自己在线
//@param
//rx    网关接收到的数据
//t_model 网关发送数据的通信方式
//t_netid 网关所在网络的id
string access_device::heart_package(string &t_model)
{
    string out = "";
    out = out + "$" + "FF" + t_model + "00" + net_id + "00" + "12" + mac + id + "@";
    return out;
}
