#pragma once
#include <iostream>
#include <string>
#include <sstream>
using namespace std;
class access_device
{
public:
    access_device(){
        mess_num = -2;      //将剩余包数量初始化为-1，后期接收到数据的时候则将其置为正直。
    }
    ~access_device() {}

    string get_id();
    string get_netid();
    string get_mac();
    bool get_op();
    string get_mess();
    char get_st();
    int get_m_num();
    char get_en();
    int get_fre();
    int get_heart();
    //修改设备的信息
    void change_op(bool &t_op);
    void change_fre(int &t_fre);
    void change_id(string &t_id);
    void change_heart(int t_heart);
    //重置设备参数
    void clear_info();
    //创建接入设备类
    void create_device(string &buf, string &num, string &net, int &fre);
    //注册函数
    string regist_device();
    //第二次注册函数，主要用于在接入设备异常掉线，由在心跳时间内上线后进行注册确认操作
    //string second_regist_device();
    //确认函数
    void confirm_device(string &t_id, string &t_netid, string &t_mac);
    //数据上传函数
    string send_device(string &rx_mess);
    //网关心跳包
    string heart_package(string &t_model);
private:
    string id;          //设备在网络中的id
    string net_id;         //设备所在网络的net_id
    string mac;         //设备的mac地址
    bool op;            //设备的开机标识
    int freque;         //设备上传数据的频率
    string message;     //接收到的数据包
    char mess_st;       //包起始符号：$
    int mess_num;       //剩余包的数量
    char mess_en;       //包结束符号：@
    int hearbeat;       //心跳
};


