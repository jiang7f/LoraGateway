#ifndef __PACKETRECEIVETHRD_H
#define __PACKETRECEIVETHRD_H

#include <pthread.h>

#include "LoraServer.h"

// ******************** Thread of Packet Receive ********************
typedef struct PacketReceiveThread PacketReceiveThread;
struct PacketReceiveThread {
  pthread_t* pId;
  void* (*vpStartRoutine)(void*);
  void (*vCreate)(PacketReceiveThread*);
};
void vPacketReceiveRun();
// 线程1构造函数
PacketReceiveThread* spNewPacketReceiveThread();

void vCreatePacketReceiveThread(PacketReceiveThread* this);
void vDestoryPacketReceiveThread(PacketReceiveThread* this);
#endif