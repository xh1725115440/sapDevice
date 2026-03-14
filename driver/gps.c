#include "gps.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    double  rcv_time;
    char    status;
    double  lat;
    char    lat_direct;
    double  lon;
    char    lon_direct;
    double  sog;
    double  cog;
    int     date;
    double  mag_variation;
    char    mag_var_direct;
    char    mode;
    int     chksum;
} gprmc_format;


int getGps(char* line,char	* _lat,char* _lon)
{

	    // 从数据读取一行
    //char line[512] = "$GPRMC,042502.00,A,2932.01902,N,10636.07161,E,0.073,,130519,,,A*79";
		memset(_lat,0x00,20);
		memset(_lon,0x00,20);
		if(strchr(line, 'V') != NULL) return 0;//长度小于60代表失败

 
    gprmc_format rmc;
 
    int chksum = 0;
    char tempstr[512];
    char finalstr[512];
 
    if ('$' != line[0]) {
        //print_error(1);
        return -1;
 
    } else {
 
        char* pch = strrchr(line, '*');
        if (pch != NULL) {
            *pch = '\0';
            rmc.mode = *(pch - 1);
            pch++;
            rmc.chksum = strtol(pch, &pch, 16);
            // printf("%X\n", chksum);
 
            if (rmc.chksum != checksum(line + 1)) {
 //               print_error(2);
                return -1;
            }
        } else {
 //           print_error(3);
            return -1;
        }
 
 
        pch = strtok(line, ",");
        if ((pch != NULL) && !strcmp(pch, "$GNRMC")) {
            // printf("%s\n", pch);    //GPRMC
 
            pch = strtok(NULL, ",");
            rmc.rcv_time = atof(pch);
 
            pch = strtok(NULL, ",");
            rmc.status = *pch;
 
            pch = strtok(NULL, ",");
            rmc.lat = atof(pch);
						strcpy(_lat,pch);
 
            pch = strtok(NULL, ",");
            rmc.lat_direct  = *pch;
 
            pch = strtok(NULL, ",");
            rmc.lon = atof(pch);
						strcpy(_lon,pch);
 
            pch = strtok(NULL, ",");
            rmc.lon_direct = *pch;
 
            pch = strtok(NULL, ",");
            rmc.sog  = atof(pch);
 
            pch = strtok(NULL, ",");
            rmc.cog  = atof(pch);
 
            pch = strtok(NULL, ",");
            rmc.date  = atoi(pch);
 
            // 一般空
            rmc.mag_variation   = 0;
            rmc.mag_var_direct  = 'W';
 
            rmc.mode = rmc.mode; // 之前已经读到
						return 1;
 
        } else {
//            print_error(4);
            return -1;
 
        }
    }
//		printf("%s\n", tempstr);
		/*
 
    // 测试重新生成 GPRMC 数据，校验值会变化
    sprintf(tempstr, "%s,%06.0f,%c,%.5f,%c,%.5f,%c,%.2f,%.1f,%06u,,,%c",
            "GPRMC",
            rmc.rcv_time,
            rmc.status,
            rmc.lat,
            rmc.lat_direct,
            rmc.lon,
            rmc.lon_direct,
            rmc.sog,
            rmc.cog,
            rmc.date,
            rmc.mode
           );
 
    //printf("%s\n", tempstr);
    chksum = checksum(tempstr);
    sprintf(finalstr, "$%s*%X", tempstr, chksum);
//    printf("%s\n", finalstr);
 
 
    // 打印 GPS坐标速度时间戳
    fprintf(stdout, "纬度\t经度\t时速(Km/H)\t时间戳\n");
//    print_gps_point(stdout, &rmc);
		*/
}
int checksum(const char* s)
{
    int c = 0;
 
    while (*s)
        c ^= *s++;
 
    return c;
}
