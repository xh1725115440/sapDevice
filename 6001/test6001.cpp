/*
* 时间：2024年5月5日
* 作用：6001无人船测试代码
* 
* 时间：2024年9月26日
* 作用：6001无人船测试代码
*/
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
返回一个字的高8位
*/
uint8_t int16_t_getH(uint16_t a)
{    //返回一个字的高8位
    uint8_t aa = (a >> 8) & 0xFF;
    return aa;
}
/*
返回一个字的低8位
*/
uint8_t int16_t_getL(uint16_t a)
{    //返回一个字的低8位
    uint8_t b = 0xff & a;
    return b;
}


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

int main()
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint16_t temp3_16, humid_16, co2_16;
    // 6001 水质传感器：溶解氧、氨氮、叶绿素、蓝绿藻、电导率、浊度、PH（1-7是水质传感器，8,9,10 大气传感器
    int slave_address[12] = { 1,2,3,4,5,6,7,8,9,10 };
    uint16_t tab_reg[512] = { 0 };
    uint8_t ship_getpacket[2048] = { 0 };
    float sensor_data[20] = {};
    float rongO = -999, NH = -999, lanlz = -999, yels = -999, diandl = -999, NTU = -999, PH = -999;
    float temp1 = -999, temp2 = -999;
    // 6001 大气传感器：CO、O3、SO2、NO2、Humid、tempAir、Light、CO2
    float CO = -999, O3 = -999, SO2 = -999, NO2 = -999, Humid = -999, tempAir = -999, Light = -999, CO2 = -999, NH3 = -999, O2 = -999;
    float pm10 = -999, pm25 = -999, pm1 = -999;
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
        cout << "开始获取传感器" << index_flag + 1 << "数据" << endl;
        modbus_set_slave(ctx1, slave_address[index_flag]); //设置从机地址
        memset(tab_reg, 0, sizeof(tab_reg));
        //开始读取数据
        switch (index_flag) {
        case 0: //从设备1 溶解氧传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取溶解氧值 04 03 00 01 CRC  *CDAB* 注意转化字节顺序
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
            c = int16_t_getH(tab_reg[0]);   //C
            d = int16_t_getL(tab_reg[0]);   //D
            a = int16_t_getH(tab_reg[1]);   //A
            b = int16_t_getL(tab_reg[1]);   //B
            //打印输出ABCD
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            //保存溶解氧
            rongO = float2decimal(a, b, c, d);
            sensor_data[0] = rongO;
            sleep(2);
            break;
        case 1: //从设备2 氨氮传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取氨氮值 05 03 00 01 CRC  *CDAB* 注意转化字节顺序
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
            c = int16_t_getH(tab_reg[0]);
            d = int16_t_getL(tab_reg[0]);
            a = int16_t_getH(tab_reg[1]);
            b = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            //保存氨氮
            NH = float2decimal(a, b, c, d);
            sensor_data[1] = NH;
            sleep(2);
            break;
        case 2: //从设备3 叶绿素传感器
            regs = modbus_read_registers(ctx1, 9728, 5, tab_reg); //对应于读取温度和叶绿素值    DCBA //0x26 00 = 9728
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
            d = int16_t_getH(tab_reg[0]);
            c = int16_t_getL(tab_reg[0]);
            b = int16_t_getH(tab_reg[1]);
            a = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            temp2 = float2decimal(a, b, c, d);
            sensor_data[2] = temp2; //保存温度
            d = int16_t_getH(tab_reg[2]);
            c = int16_t_getL(tab_reg[2]);
            b = int16_t_getH(tab_reg[3]);
            a = int16_t_getL(tab_reg[3]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            yels = float2decimal(a, b, c, d);
            sensor_data[3] = yels;  //保存叶绿素
            sleep(2);
            break;
        case 3: //从设备4 蓝绿藻传感器
            regs = modbus_read_registers(ctx1, 9728, 5, tab_reg); //对应于读取温度和蓝绿藻值 07 03 26 00 CRC  *DCBA* 注意转化字节顺序
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
            d = int16_t_getH(tab_reg[0]);
            c = int16_t_getL(tab_reg[0]);
            b = int16_t_getH(tab_reg[1]);
            a = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            temp1 = float2decimal(a, b, c, d);
            sensor_data[4] = temp1; //保存温度
            d = int16_t_getH(tab_reg[2]);
            c = int16_t_getL(tab_reg[2]);
            b = int16_t_getH(tab_reg[3]);
            a = int16_t_getL(tab_reg[3]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            lanlz = float2decimal(a, b, c, d);
            sensor_data[5] = lanlz; //保存蓝绿藻值
            sleep(2);
            break;
        case 4: //从设备5 电导率传感器
            regs = modbus_read_registers(ctx1, 0, 2, tab_reg); //对应于读取电导率值 03 03 00 00 CRC  *CDAB* 注意转化字节顺序
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
            c = int16_t_getH(tab_reg[0]);
            d = int16_t_getL(tab_reg[0]);
            a = int16_t_getH(tab_reg[1]);
            b = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            //保存电导率
            diandl = float2decimal(a, b, c, d);
            sensor_data[6] = diandl;
            sleep(2);
            break;
        case 5: //从设备6 浊度传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取浊度值    CDAB 
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
            d = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[1]);
            a = int16_t_getH(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            NTU = float2decimal(a, b, c, d);
            sensor_data[7] = NTU;  //保存浊度
            sleep(2);
            break;
        case 6: //从设备7 PH传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取PH值   *CDAB* 注意转化字节顺序
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
            d = int16_t_getL(tab_reg[0]);
            c = int16_t_getH(tab_reg[0]);
            b = int16_t_getL(tab_reg[1]);
            a = int16_t_getH(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            //保存电导率
            PH = float2decimal(a, b, c, d);
            sensor_data[8] = PH;
            sleep(2);
            break;
        case 7: //从设备8 四合一，湿度、温度、光照、CO2
            regs = modbus_read_registers(ctx1, 0, 5, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取传感器" << index_flag + 1 << "数据失败" << endl;
                a = 0;
                b = 0;
                c = 0;
                d = 0;
                sleep(4);
                break;
            }
            Humid = tab_reg[0];
            sensor_data[9] = Humid * 0.1;

            tempAir = tab_reg[1];
            sensor_data[10] = tempAir * 0.1;

            CO2 = tab_reg[2];
            sensor_data[11] = CO2;


            a = int16_t_getH(tab_reg[3]);
            b = int16_t_getL(tab_reg[3]);
            c = int16_t_getH(tab_reg[4]);
            d = int16_t_getL(tab_reg[4]);
            Light = float2decimal(a, b, c, d);
            sensor_data[12] = Light;
            sleep(2);
            break;
        case 8: //从设备9 NH3
            regs = modbus_read_registers(ctx1, 0, 1, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取气体传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            NH3 = tab_reg[0];
            cout << NH3 << endl;
            sensor_data[13] = NH3;
            sleep(2);
            break;
        case 9: //从设备10 O2
            regs = modbus_read_registers(ctx1, 0, 1, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取气体传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            O2 = tab_reg[0];
            cout << O2 << endl;
            O2 = O2 / 10;
            sensor_data[14] = O2;
            sleep(2);
            break;
        }
        if (index_flag == 10) {
            index_flag = -1;
            cout << "--------------------------------" << endl;
            cout << "获取一轮后的数据如下：" << endl;
            printf("溶解氧：%f\n氨氮：%f\n蓝绿藻：%f\n叶绿素：%f\n电导率：%f\n浊度：%f\nPH：%f\n",
                sensor_data[0], sensor_data[1], sensor_data[3], sensor_data[5], sensor_data[6], sensor_data[7], sensor_data[8]);
            printf("水温1：%f\n水温2：%f\n",
                sensor_data[2], sensor_data[4]);
            printf("环境湿度：%f %RH\n环境温度：%f ℃\nCO2：%f ppm\n光照:  %f Lux\nNH3：%f ppm\nO2：%f %VOL\n",
                sensor_data[9], sensor_data[10], sensor_data[11], sensor_data[12], sensor_data[13], sensor_data[14]);
            cout << "--------------------------------" << endl;

            float rongO = -999, NH = -999, Cu = -999, Cd = -999, lanlz = -999, yels = -999, diandl = -999, NTU = -999, PH = -999;
            float temp1 = -999, temp2 = -999; tempAir = -999;
            float temp3 = -999, humid = -999, co2 = -999, pm10 = -999, pm25 = -999, pm1 = -999; NH3 = -999; O2 = -999;
        }
        index_flag++;
    }
}
