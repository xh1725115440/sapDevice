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

/*
共用体获取32位精度浮点数
成员共享相同的内存空间
*/
union Tdata
{
    float testData_float;
    unsigned char testArray[4];
}TData;

int main()
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    // 设备地址码
    int slave_address[1] = { 1 };
    uint16_t tab_reg[512] = { 0 };
    float sensor_data[10] = {};
    float  humid = -999, temp = -999, light = -999, CO2 = -999;
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
            regs = modbus_read_registers(ctx1, 0, 5, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                a = 0;
                b = 0;
                c = 0;
                d = 0;
                cout << "读取传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            cout << tab_reg[0] << endl;
            humid = tab_reg[0];
            sensor_data[0] = humid * 0.1;

            cout << tab_reg[1] << endl;
            temp = tab_reg[1];
            sensor_data[1] = temp * 0.1 ;

            cout << tab_reg[2] << endl;
            CO2 = tab_reg[2];
            sensor_data[2] = CO2;


            a = int16_t_getH(tab_reg[3]);
            b = int16_t_getL(tab_reg[3]);
            c = int16_t_getH(tab_reg[4]);
            d = int16_t_getL(tab_reg[4]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            //保存光照值
            TData.testArray[3] = d;
            TData.testArray[2] = c;
            TData.testArray[1] = b;
            TData.testArray[0] = a;
            //light = TData.testData_float;
            light  = float2decimal(a, b, c, d);
            sensor_data[3] = light;
            sleep(4);
            break;
       
        }
        if (index_flag == 0) {
            index_flag = -1;
            cout << "--------------------------------" << endl;
            cout << "获取一轮后的数据如下：" << endl;
            printf("环境湿度：%f %RH\n环境温度：%f ℃\nCO2：%f ppm\n光照:  %fhlux\n",
                sensor_data[0], sensor_data[1], sensor_data[2]), sensor_data[3];
            cout << "--------------------------------" << endl;

            humid = -999, temp = -999, light = -999, CO2 = -999;
        }
        index_flag++;
    }
    return 0;
}
