#include "packet_show.h"


void packet_show(int pos, uint8_t *buf)
{
    int j;
    //pos是从串口读数据返回的读取的字符长度
    for(j = 0; j < pos; j++)
    {
        //printf("T");
        printf("%c", buf[j]);
       // printf("%c", buf[j]);
    }
    printf("\n");
    return;
}
