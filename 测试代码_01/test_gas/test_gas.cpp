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

int main()
{
    // CO O3 SO2 NO2
    int slave_address[4] = { 1,2,3,4 };
    uint16_t tab_reg[512] = { 0 };
    float sensor_data[10] = {};
    float CO = -999, O3 = -999, SO2 = -999, NO2 = -999;
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
        cout << "<--------气体传感器连接成功-------->" << endl;
    }
    while (1) {
        cout << "开始获取气体传感器" << index_flag+1 << "数据" << endl;
        modbus_set_slave(ctx1, slave_address[index_flag]); //设置从机地址
        memset(tab_reg, 0, sizeof(tab_reg));
        //开始读取数据
        switch (index_flag) {
        case 0: //从设备1 CO
            regs = modbus_read_registers(ctx1, 257, 1, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取气体传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            CO = tab_reg[0];
            cout << CO << endl;
            CO = CO * 0.001;
            sensor_data[0] = CO;
            sleep(2);
            break;
        case 1: //从设备2 O3
            regs = modbus_read_registers(ctx1, 257, 1, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取气体传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            O3 = tab_reg[0];
            cout << O3 << endl;
            O3 = O3 * 0.001;
            sensor_data[1] = O3;
            sleep(2);
            break;
        case 2: //从设备3 SO2
            regs = modbus_read_registers(ctx1, 257, 1, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取气体传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            SO2 = tab_reg[0];
            cout << SO2 << endl;
            SO2 = SO2 * 0.001;
            sensor_data[2] = SO2;
            sleep(2);
            break;
        case 3: //从设备4 NO2
            regs = modbus_read_registers(ctx1, 257, 1, tab_reg);
            if (regs == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                cout << "读取气体传感器" << index_flag + 1 << "数据失败" << endl;
                sleep(4);
                break;
            }
            NO2 = tab_reg[0];
            cout << NO2 << endl;
            NO2 = NO2 * 0.001;
            sensor_data[3] = NO2;
            sleep(2);
            break;
        }
        if (index_flag == 3) {
            index_flag = -1;
            cout << "--------------------------------" << endl;
            cout << "获取一轮后的数据如下：" << endl;
            printf("CO：%f ppm\nO3：%f ppm\nSO2：%f ppm\nNO2：%f ppm\n",
                sensor_data[0], sensor_data[1], sensor_data[2], sensor_data[3]);
            cout << "--------------------------------" << endl;

            CO = -999, O3 = -999, SO2 = -999, NO2 = -999;
        }
        index_flag++;
    }
    return 0;
}
