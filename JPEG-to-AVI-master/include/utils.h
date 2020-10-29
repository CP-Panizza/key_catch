//
// Created by cmj on 2020/10/26.
//

#ifndef JPEG_TO_AVI_MASTER_UTILS_H
#define JPEG_TO_AVI_MASTER_UTILS_H

typedef unsigned short int uint16;
typedef unsigned long int uint32;

// 短整型大小端互换
#define BigLittleSwap16(A)  ((((uint16)(A) & 0xff00) >> 8) | \
                            (((uint16)(A) & 0x00ff) << 8))

 // 长整型大小端互换
#define BigLittleSwap32(A)  ((((uint32)(A) & 0xff000000) >> 24) | \
                            (((uint32)(A) & 0x00ff0000) >> 8) | \
                            (((uint32)(A) & 0x0000ff00) << 8) | \
                            (((uint32)(A) & 0x000000ff) << 24))



 // 本机大端返回1，小端返回0
int checkCPUendian();
// 模拟htonl函数，本机字节序转网络字节序
unsigned long int t_htonl(unsigned long int h);
// 模拟ntohl函数，网络字节序转本机字节序
unsigned long int t_ntohl(unsigned long int n);
// 模拟htons函数，本机字节序转网络字节序
unsigned short int t_htons(unsigned short int h);
// 模拟ntohs函数，网络字节序转本机字节序
unsigned short int t_ntohs(unsigned short int n);

#endif //JPEG_TO_AVI_MASTER_UTILS_H
