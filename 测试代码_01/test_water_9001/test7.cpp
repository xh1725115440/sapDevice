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
共用体获取32位精度浮点数
成员共享相同的内存空间
*/
union Tdata
{
    float testData_float;
    unsigned char testArray[4];
}TData;

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

//十六进制转十进制
int Hex_Conversion_Dec(int aHex)
{
    long Dec = 0;
    int temp = 0;
    int count = 0;
    int DecArr[100] = { 0 };
    while (0 != aHex)//循环直至aHex的商为零
    {
        cout << aHex % 16 << endl;//测试aHex%16取余得到得是不是一位
        temp = aHex;
        aHex = aHex / 16;//求商
        temp = temp % 16;//取余
        DecArr[count++] = temp;//余数存入数组
    }
    int j = 0;
    for (int i = 0; i < count; i++)
    {
        if (i < 1)
        {
            Dec = Dec + DecArr[i];
        }
        else
        {
            //16左移4位即16²，左移8位即16³、以此类推。
            Dec = (Dec + (DecArr[i] * (16 << j)));
            j += 4;
        }
    }
    cout << Dec << endl;
    return 0;
}

int main()
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint16_t temp3_16, humid_16, co2_16;
    //溶解氧、氨氮、铜离子、镉离子、蓝绿藻、叶绿素、电导率、COD 
    int slave_address[9] = { 1,2,3,4,5,6,7,8,9 };
    modbus_t* ctx1 = NULL;
    uint16_t tab_reg[512] = { 0 };
    float sensor_data[15] = {}; 
    float rongO = -999, NH = -999, Cu = -999, Cd = -999, lanlz = -999, yels = -999, diandl = -999, TDS = -999;
    float temp1 = -999, temp2 = -999;
    float COD = -999, turb = -999;
    int regs = 0;
    int index_flag = 0;
    //RTU
    ctx1 = modbus_new_rtu("/dev/ttymxc2", 9600, 'N', 8, 1);
    modbus_rtu_set_serial_mode(ctx1, MODBUS_RTU_RS485);
    modbus_rtu_set_rts(ctx1, MODBUS_RTU_RTS_DOWN);
    modbus_set_debug(ctx1, TRUE);
    //建立连接
    if (modbus_connect(ctx1) == -1) {
        printf("connection failed");
        modbus_strerror(errno);
        modbus_free(ctx1);
        return false;
    }
    else {
        cout << "数景艇传感器连接成功!" << endl;
    }
    while (1) {
        cout << "开始获取数景艇传感器" << index_flag+1 << "数据" << endl;
        modbus_set_slave(ctx1, slave_address[index_flag]); //设置从机地址
        memset(tab_reg, 0, sizeof(tab_reg));
        //开始读取数据
        switch (index_flag) {
        case 0: //从设备1 溶解氧传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取溶解氧值 04 03 00 01 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a = 0;
                b = 0;
                c = 0;
                d = 0;
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
            TData.testArray[3] = d;
            TData.testArray[2] = c;
            TData.testArray[1] = b;
            TData.testArray[0] = a;
            //rongO = TData.testData_float;
            rongO = float2decimal(a, b, c, d);
            sensor_data[0] = rongO;
            break;
        case 1: //从设备2 氨氮传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取氨氮值 05 03 00 01 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a = 0;
                b = 0;
                c = 0;
                d = 0;
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
            TData.testArray[3] = d;
            TData.testArray[2] = c;
            TData.testArray[1] = b;
            TData.testArray[0] = a;
            //NH = TData.testData_float;
            NH = float2decimal(a, b, c, d);
            sensor_data[1] = NH;
            break;
        case 2: //从设备3 铜离子传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取铜离子值 05 03 00 01 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a = 0;
                b = 0;
                c = 0;
                d = 0;
            }
            c = int16_t_getH(tab_reg[0]);
            d = int16_t_getL(tab_reg[0]);
            a = int16_t_getH(tab_reg[1]);
            b = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            //保存铜离子值
            TData.testArray[3] = d;
            TData.testArray[2] = c;
            TData.testArray[1] = b;
            TData.testArray[0] = a;
            //Cu = TData.testData_float;
            Cu = float2decimal(a, b, c, d) * 1000;
            sensor_data[2] = Cu;  
            break;
        case 3: //从设备4 镉离子传感器
            regs = modbus_read_registers(ctx1, 1, 2, tab_reg); //对应于读取镉离子值 05 03 00 01 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a = 0;
                b = 0;
                c = 0;
                d = 0;
            }
            c = int16_t_getH(tab_reg[0]);
            d = int16_t_getL(tab_reg[0]);
            a = int16_t_getH(tab_reg[1]);
            b = int16_t_getL(tab_reg[1]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            //保存镉离子值
            TData.testArray[3] = d;
            TData.testArray[2] = c;
            TData.testArray[1] = b;
            TData.testArray[0] = a;
            //Cd = TData.testData_float;
            Cd = float2decimal(a, b, c, d);
            sensor_data[3] = Cd;
            break;
        case 4: //从设备5 蓝绿藻传感器
            regs = modbus_read_registers(ctx1, 9728, 4, tab_reg); //对应于读取温度和蓝绿藻值 07 03 26 00 CRC  *DCBA* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a = 0;
                b = 0;
                c = 0;
                d = 0;
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
            lanlz = float2decimal(a, b, c, d) * 0.0001;
            sensor_data[5] = lanlz; //保存蓝绿藻值
            break;
        case 5: //从设备6 叶绿素传感器
            regs = modbus_read_registers(ctx1, 9728, 4, tab_reg); //对应于读取温度和叶绿素值    DCBA 
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a = 0;
                b = 0;
                c = 0;
                d = 0;
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
            sensor_data[6] = temp2; //保存温度
            d = int16_t_getH(tab_reg[2]);
            c = int16_t_getL(tab_reg[2]);
            b = int16_t_getH(tab_reg[3]);
            a = int16_t_getL(tab_reg[3]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            yels = float2decimal(a, b, c, d);
            sensor_data[7] = yels;  //保存叶绿素
            break;
        case 6: //从设备7 电导率传感器
            regs = modbus_read_registers(ctx1, 0, 2, tab_reg); //对应于读取电导率值 03 03 00 00 CRC  *CDAB* 注意转化字节顺序
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                //return false;
                a = 0;
                b = 0;
                c = 0;
                d = 0;
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
            TData.testArray[3] = d;
            TData.testArray[2] = c;
            TData.testArray[1] = b;
            TData.testArray[0] = a;
            //diandl = TData.testData_float;
            diandl = float2decimal(a, b, c, d) * 1000;
            sensor_data[8] = diandl;
            c = int16_t_getH(tab_reg[6]);
            d = int16_t_getL(tab_reg[6]);
            a = int16_t_getH(tab_reg[7]);
            b = int16_t_getL(tab_reg[7]);
            printf("%x", a);
            printf("%x", b);
            printf("%x", c);
            printf("%x\n", d);
            //保存TDS
            TDS = float2decimal(a, b, c, d);
            sensor_data[9] = TDS;
            break;
        case 7: //从设备8 在线COD传感器
            regs = modbus_read_registers(ctx1, 0, 6, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
            }
            COD = tab_reg[0] * 0.1;
            sensor_data[10] = COD; //保存COD

            turb = tab_reg[4] * 0.1;
            sensor_data[11] = turb; //保存浊度
            sleep(1);
            break;
        }
        if (index_flag == 7) {
            index_flag = -1;
            cout << "--------------------------------" << endl;
            cout << "获取一轮后的数据如下：" << endl;
            printf("溶解氧：%f mg/L\n氨氮：%f mg/L\n铜离子：%f μg/L\n镉离子：%f\n蓝绿藻：%f 万cells/mL\n叶绿素：%f mg/L\n电导率：%f μS/cm\nTDS：%f mg/L\nCOD: %f mg/L\n浊度: %f NTU\n",
                sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3], sensor_data[5], sensor_data[7], sensor_data[8], sensor_data[9],sensor_data[10], sensor_data[11]);
            printf("温度1：%f ℃\n温度2：%f ℃\n",
                sensor_data[4], sensor_data[6]);
            cout << "--------------------------------" << endl;
        }
        index_flag++;
    }
}
