#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <iomanip>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>
#include "/home/wp/guoxiang/apDevice/libmodbus/include/modbus/modbus.h"
using namespace std;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;

/*
*利用函数获取32位精度浮点数
*/
float float2decimal(long int byte0, long int byte1, long int byte2, long int byte3)
{

    long int realbyte0, realbyte1, realbyte2, realbyte3;
    char S;
    long int E, M;
    float D;
    realbyte0 = byte0;
    realbyte1 = byte1;
    realbyte2 = byte2;
    realbyte3 = byte3;
    if ((realbyte0 & 0x80) == 0) {
        S = 0;//正数
    }
    else {
        S = 1;
    }
    E = ((realbyte0 << 1) | (realbyte1 & 0x80) >> 7) - 127;
    M = ((realbyte1 & 0x7f) << 16) | (realbyte2 << 8) | realbyte3;
    D = pow(-1, S) * (1.0 + M / pow(2, 23)) * pow(2, E);
    return D;
}

/*
*返回一个字的高8位
*/
uint8_t int16_t_getH(uint16_t a)
{    //返回一个字的高8位
    uint8_t aa = (a >> 8) & 0xFF;
    return aa;
}
/*
*返回一个字的低8位
*/
uint8_t int16_t_getL(uint16_t a)
{    //返回一个字的低8位
    uint8_t b = 0xff & a;
    return b;
}

int main()
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    // 设备地址码
    int slave_address[2] = { 12,13 };
    uint16_t tab_reg[512] = { 0 };
    float sensor_data[10] = {};
    float fengx = -999, fengs = -999, rain = -999, light = -999, temp = -999, humid = -999, pressure = -999, noise = -999;
    int regs = 0;
    int index_flag = 0;
    //RTU
    modbus_t* ctx1 = NULL;
    ctx1 = modbus_new_rtu("/dev/ttymxc2", 9600, 'N', 8, 1);
    modbus_rtu_set_serial_mode(ctx1, MODBUS_RTU_RS485);
    modbus_rtu_set_rts(ctx1, MODBUS_RTU_RTS_DOWN);
    modbus_set_debug(ctx1, TRUE);
    //建立连接
    if (modbus_connect(ctx1) == -1) {
        fprintf(stderr, "Connection failed:%s\n", modbus_strerror(errno));
        modbus_free(ctx1);
        return false;
    }
    else {
        cout << "<--------传感器连接成功-------->" << endl;
    }
    while (1) {
        cout << "开始获取传感器" << index_flag+1 << "数据" << endl;
        modbus_set_slave(ctx1, slave_address[index_flag]); //设置从机地址
        memset(tab_reg, 0, sizeof(tab_reg));
        //开始读取数据
        switch (index_flag) {
        case 0: //从设备1
            regs = modbus_read_registers(ctx1, 0, 7, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            cout << tab_reg[0] << endl;
            fengx = tab_reg[0];
            sensor_data[0] = fengx;

            cout << tab_reg[1] << endl;
            fengs = tab_reg[1];
            sensor_data[1] = fengs * 0.1;

            cout << tab_reg[2] << endl;
            rain = tab_reg[2];
            sensor_data[2] = rain;

            cout << tab_reg[3] << endl;
            light = tab_reg[3];
            sensor_data[3] = light * 0.1;

            cout << tab_reg[4] << endl;
            temp = tab_reg[4];
            sensor_data[4] = temp * 0.1;

            cout << tab_reg[5] << endl;
            humid = tab_reg[5];
            sensor_data[5] = humid * 0.1;

            cout << tab_reg[6] << endl;
            pressure = tab_reg[6];
            sensor_data[6] = pressure * 0.1;

            sleep(2);
            break;
        case 1: //从设备2
            regs = modbus_read_registers(ctx1, 0, 1, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            cout << tab_reg[0] << endl;
            noise = tab_reg[0];
            sensor_data[7] = noise * 0.1;
            sleep(2);
            break;
        }
        if (index_flag == 1) {
            index_flag = -1;
            cout << "--------------------------------" << endl;
            cout << "获取一轮后的数据如下：" << endl;
            printf("风向：%f °\n风速：%f m/s\n雨量：%f mm\n光照度：%f hlux\n环境温度：%f ℃\n环境湿度：%f %RH\n气压：%f hPa\n噪声：%f dB\n",
                sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3], sensor_data[4], sensor_data[5], sensor_data[6], sensor_data[7]);
            cout << "--------------------------------" << endl;

            fengx = -999, fengs = -999, rain = -999, light = -999, temp = -999, humid = -999, pressure = -999, noise = -999;
        }
        index_flag++;
    }
    return 0;
}
