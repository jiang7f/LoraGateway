#ifndef __PACKET_H
#define __PACKET_H

#include "LoraServer.h"
#include "loragw_hal.h"

typedef struct lgw_pkt_tx_s TxData;
// 终端节点发送过来的定义为Data
typedef struct lgw_pkt_rx_s RxData;
// DataArray为长度16的Data数组
typedef RxData DataArray[BUFFERMAX];
// Data数组+数组长度 定义为Packet
typedef struct Packet Packet;
struct Packet {
  DataArray rxPkt;
  int iLen;
  /*
   * 数据类型,允许一个包内的数据类型不同
   * e 错误包
   * s 分组请求包
   * t 温度包
   * o 其他
   */
  char cType[BUFFERMAX];
  char cAccessMethod;                      // 数据接入方式
};
// 数据包处理每个数据
void vProcessOneByOne(Packet* this);
// 数据包析构函数
void vDestoryPacket(Packet* this);
// 数据包构造函数
Packet* spNewPacket(DataArray rxPct, int len);

#endif