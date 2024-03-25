#include "PacketReceiveThrd.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "LoraGateway.h"
#include "Packet.h"
#include "UnpackingThrd.h"
// ******************** 线程PacketReceive ********************
PacketReceiveThread* spNewPacketReceiveThread() {
  PacketReceiveThread* this =
      (PacketReceiveThread*)malloc(sizeof(PacketReceiveThread));
  this->pId = (pthread_t*)malloc(sizeof(pthread_t));
  this->vpStartRoutine = (void*)vPacketReceiveRun;
  this->vCreate = vCreatePacketReceiveThread;
  return this;
}
void vPacketReceiveRun() {
  printf("PacketReceiveRun start\n");
  // 接收收到数量
  int iNbData;
  // 接收缓冲区(数组)
  DataArray aRxPktBuffer;
  while (loraGateway->bCheckNoSignal()) {
    // LoraGateway从Lora缓冲区接收数据，返回收到Data的个数
    iNbData = loraGateway->iRxFromLora(aRxPktBuffer);
    if (0 == iNbData) {
      loraGateway->vWaitMs(iWaitMs);  // 等待iWaitMs毫秒
    } else {
      // 生成数据包对象，这个地方可能需要加内存池来优化
      Packet* spRxPkts = spNewPacket(aRxPktBuffer, iNbData);
      // 交给数据包处理线程
      UnpackingThread* unpacingThread = spNewUnpackingThread(spRxPkts);
      unpacingThread->vCreate(unpacingThread);
      // 数据包对象析构在处理线程中执行
    }
  }
};

void vCreatePacketReceiveThread(PacketReceiveThread* this) {
  if (0 != pthread_create(this->pId, NULL, this->vpStartRoutine, NULL)) {
    printf("Create PacketReceiveThread error!\n");
  } else {
#ifdef SHOW_SUCCESS
    printf("Create PacketReceiveThread successfully\n");
#endif
  }
  return;
};
void vDestoryPacketReceiveThread(PacketReceiveThread* this) {
  pthread_join(*(this->pId), NULL);
#ifdef SHOW_DESTORY
  printf("\nPacketReceiveThread Destory");
#endif
  printf("\n");
  free(this->pId);
  free(this);
}
