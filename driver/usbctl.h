#ifndef _USBCTL_H_
#define _USBCTL_H_

#include <stddef.h>
#include <stdint.h>
// 串口/端口打开[阻塞方式]
// @param
// @comport 需要打开的USB端口号 < 如：ttyUSB0和ttyUSB1 comport分别为0和1>
// @return  打开失败返回< -1 >
//          打开成功返回对应的< 文件描述符fd>0 >
int open_port(int comport);

// 串口配置
// @param
// @fd      文件描述符fd
// @nSpeed  波特率选项< 2400 4800 9600 115200 460800 > 若不为上述选项，则自动设为9600
// @nBits   数据位数选项 < 7 8 >
// @nEvent  奇偶检验为选项 < o e n>
// @nStop   设置停止位 < 1 2 >
// @return  错误代码
int set_opt(int fd, int nSpeed, int nBits, uint8_t nEvent, int nStop);

// 串口配置 主要用于485端口的端口配置
// @param
// @fd      文件描述符fd
// @nSpeed  波特率选项< 2400 4800 9600 115200 460800 > 若不为上述选项，则自动设为9600
// @nBits   数据位数选项 < 7 8 >
// @nEvent  奇偶检验为选项 < o e n>
// @nStop   设置停止位 < 1 2 >
// @falg    是否开启硬件流控制 1开启 0不开启
// @return  错误代码
//
int set_opt(int fd, int nSpeed, int nBits, uint8_t nEvent, int nStop, int flag);
// 写串口
// @param
// @fd      文件描述符fd
// @buf     需写入的缓存首地址（需发送的数据/组）
// @len     写入的缓存大小 单位KB字节
// @return  失败返回< -1 >
//          成功返回已发送的字节长度
int write_port(int fd, const void *buf, size_t len);

// 写串口
// @param
// @fd      文件描述符fd
// @buf     读取得数据存储的缓存首地址
// @len   读取的数据大小 单位KB字节 必须小于给出的缓存大小
// @return  失败返回< -1 >
//          成功返回已读取的字节长度
int read_port(int fd, void *buf, size_t len);

// 读后写串口
// @param
// @fd      文件描述符fd
// @wbuf    需写入的缓存首地址（需发送的数据/组）
// @wlen    写入的缓存大小 单位KB字节
// @rbuf    读取得数据存储的缓存首地址
// @rlen    读取的数据大小 单位KB字节 必须小于给出的缓存大小
// @return  失败返回< -1 >
//          成功返回已读取的字节长度
int write_read(int fd, void *wbuf, size_t wlen, void *rbuf, size_t rlen);


#endif
