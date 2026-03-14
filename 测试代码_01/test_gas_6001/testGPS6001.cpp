/*
* 时间：2024年5月5日
* 作用：6001无人船测试代码
* 
* 时间：2024年9月27日
* 作用：6001无人船测试代码，增加GPS
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
#include <thread>                       //  线程相关
#include <pthread.h>                    //  多线程相关
#include <mutex>                        //  线程中的同步和互斥操作
// 提供了串口操作的接口
#include <termios.h>
#include <unistd.h>  // for close() function
#include <fcntl.h>   // for file control options like O_RDWR

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

/**
*串口设置函数：例(fd1, 115200, 8, 'N', 1);
*参数：
*fd:串口设备节点
*nSpeed:波特率
*nBits:数据位
*nEvent:校验位
*nStop:停止位
*返回值：成功 -- 0
*		 失败 -- -1
*/
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;
    if (tcgetattr(fd, &oldtio) != 0) {
        perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch (nEvent)
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }

    switch (nSpeed)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("com set error");
        return -1;
    }
    //	printf("set done!\n\r");
    return 0;
}

string gps_lat = "NFFFF";
string gps_lon = "EFFFFF";       //东经 //存储GPS经纬度
mutex mutex_gps;                //互斥锁，锁住gps

/**
*   GPS数据处理
*   @param *line 需要处理得数据
*   @param *lat  纬度
*   @param *flag_lat  标志 E
*   @param *lon  经度
*   @param *flag_lon  标志 N
*   @return  -1 失败  0 成功
*
*/
int process_gps(char* line, char* lat, char* flag_lat, char* lon, char* flag_lon)
{
    char* token;
    token = strstr(line, "V");   //去掉不需要的数据
    if (token != NULL)
    {
        return -1;
    }
    token = strtok(line, ",");
    if (token != NULL && !strcmp(token, "$GNRMC"))
    {
        char* time = strtok(NULL, ",");
        char* a = strtok(NULL, ",");
        char* nd = strtok(NULL, ",");
        char* n = strtok(NULL, ",");
        char* ed = strtok(NULL, ",");
        char* e = strtok(NULL, ",");
        strcpy(lat, nd);
        strcpy(flag_lat, n);
        strcpy(lon, ed);
        strcpy(flag_lon, e);
    }
    else {
        return -1;

    }
    return 0;
}
/**
*   GPS数据获取
*   @return
*
*/
void* GET_GPS(void* arg)
{
    // cout << "================================" << endl;
    //首先开启gos获取脚本
#ifdef DEBUG
    cout << "Start thread：GET_GPS()" << endl;
#endif
    system("/home/root/gps_test.sh &");
    sleep(60);
    int ret;
    uint8_t dev[] = "/dev/ttymxc7";   //gps设备描述符
    char buf[300];
    char f_lat[5];
    char f_lon[5];
    char lat[10];
    char lon[10];
    int fd;         //接收GPS数据的串口
    int i = 0;
    fd = open((const char*)dev, O_RDWR | O_NOCTTY); // O_NOCTTY不将该文件作为进程的控制终端
    if (-1 == fd)
    {
#ifdef DEBUG
        cout << "Open GPS device error!" << endl;
#endif
    }
    else {
#ifdef DEBUG
        cout << "Open GPS device success!" << endl;
#endif
        ret = set_opt(fd, 9600, 8, 'n', 1);
        int flag = 0;
        int count = 1;
        char in;
        int i = 0;
        int flag_read = 0;
        while (1)
        {
            while (1)
            {
                if (read(fd, &in, 1) == 0)break;
                if (in != '$')
                {
                    if (0 == flag_read)continue;
                    else if (in == '\n' || in == '*')
                    {
                        flag_read = 0;
                        i = 0;
                        break;
                    }
                    else if (flag_read = 1 && in != '\n')buf[i++] = in;
                }
                else if (in == '$' && flag_read == 0)
                {
                    flag_read = 1;
                    i = 0;
                    buf[i++] = in;
                }
            }
#ifdef DEBUG
            //cout << "GPS: " << buf << endl;
#endif 
            flag = process_gps(buf, lat, f_lat, lon, f_lon);
            if (-1 == flag)
            {
#ifdef DEBUG
                //cout << "GPS analyze error" << endl;
#endif          

                //N2932,E10636
                // mutex_gps.lock();
                // gps_lat = "NFFFF";   //纬度 北纬
                // gps_lon = "EFFFFF";  //经度   东经
                // mutex_gps.unlock();
                continue;

            }
            else {
#ifdef DEBUG
                cout << "lat: " << lat << " f_lat: " << f_lat << endl;  // 北纬
                cout << "lon: " << lon << " f_lon: " << f_lon << endl;  // 东经
                cout << "lat: " << lat << " lon: " << lon << " count: " << count << endl;
                printf("%c%c度%c%c分%s %c%c%c度%c%c分%s\n", lat[0], lat[1], lat[2], lat[3], f_lat, lon[0], lon[1], lon[2], lon[3], lon[4], f_lon);
#endif
                //lon是经度 lat是纬度
                string s_lon = "";
                s_lon = s_lon + f_lon + lon; //经度
                s_lon = s_lon + " " + lon[0] + lon[1] + lon[2] + lon[3] + lon[4] + lon[6] + lon[7] + lon[8] + lon[9] + lon[10];
                string s_lat = "";
                s_lat = s_lat + f_lat + lat;   //纬度
                s_lat = s_lat + lat[0] + lat[1] + lat[2] + lat[3] + lat[5] + lat[6] + lat[7] + lat[8] + lat[9];

                cout << "------------------------------------------------" << endl;
                cout << s_lon << endl;
                cout << s_lat << endl;
                cout << "------------------------------------------------" << endl;

                mutex_gps.lock();
                gps_lat = s_lat;
                gps_lon = s_lon;
                mutex_gps.unlock();

            }
            sleep(2);
        }
    }
}


int main()
{
    pthread_t tid_gps;
    pthread_create(&tid_gps, NULL, GET_GPS, NULL);    //开启GPS获取线程
    cout << " Start GET_GPS pthread Now" << endl;
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

    // GPS
    string accessdev_gps = gps_lat + gps_lon;
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
        //一直更新GPS信息数据
        cout << "update GPS Data " << endl;
        accessdev_gps = gps_lat + gps_lon;
        pthread_create(&tid_gps, NULL, GET_GPS, NULL);
        cout << "Raw_GPS_data is " << accessdev_gps << endl;
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
