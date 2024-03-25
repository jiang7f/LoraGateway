#include "UnpackingThrd.h"

#include <stdio.h>
#include <stdlib.h>

#include "LoraGateway.h"
#include "UnpackingOperator.h"

void fun() { printf("fun successfully\n"); };
// ******************** Thread of Unpacking ********************
UnpackingThread* spNewUnpackingThread(Packet* packet) {
  UnpackingThread* this = (UnpackingThread*)malloc(sizeof(UnpackingThread));
  this->pId = (pthread_t*)malloc(sizeof(pthread_t));
  this->vpStartRoutine = (void*)vUnpackingRun;
  this->argv[0] = (void*)this;
  this->argv[1] = (void*)packet;  // 这个包需要在ThreadPD中析构
  this->vpArg = (void*)this->argv;
  this->vCreate = vCreateUnpackingThread;
  return this;
}
void vUnpackingRun(void* argv[]) {
  Packet* pPacket = (Packet*)argv[1];
  pthread_detach(pthread_self());
  int iTime = loraGateway->iGetSysTime();
#ifdef SHOW_MORE
  printf("----%d时处理%d个包----\n", iTime, pPacket->iLen);
#endif

#ifdef TEST
  for (int i = 0; i < BUFFERMAX; ++i) {
    switch (pPacket->rxPkt[i].payload[10]) {
      case 'a':
        iReceTimesA++;
        iSendTimesA = pPacket->rxPkt[i].payload[8];
        printf("收到一个A包 编号为%d\n", iSendTimesA);
        break;
      case 'b':
        iReceTimesB++;
        iSendTimesB = pPacket->rxPkt[i].payload[8];
        printf("收到一个B包 编号为%d\n", iSendTimesB);
        break;
      case 'c':
        iReceTimesC++;
        iSendTimesC = pPacket->rxPkt[i].payload[8];
        printf("收到一个C包 编号为%d\n", iSendTimesC);
        break;
      case 'd':
        iReceTimesD++;
        iSendTimesD = pPacket->rxPkt[i].payload[8];
        printf("收到一个D包 编号为%d\n", iSendTimesD);
        break;
    }
  }
#endif
  unpackingOperator->vDo(pPacket);
  vDestoryUnpackingThread((UnpackingThread*)argv[0]);
};
// 解包线程初始化
void vCreateUnpackingThread(UnpackingThread* this) {
  if (0 != pthread_create(this->pId, NULL, this->vpStartRoutine, this->vpArg)) {
    printf("Create UnpackingThread error!\n");
  } else {
#ifdef SHOW_SUCCESS
    printf("Create UnpackingThread successfully\n");
#endif
  }
  return;
};
void vDestoryUnpackingThread(UnpackingThread* this) {
  vDestoryPacket((Packet*)this->argv[1]);  // 析构掉这条线程解包的数据包对象
  // 已pthread_detach,会自动回收线程资源,但要考虑内存泄露(需要进行free)
  // pthread_join(*(this->pId), NULL);
  free(this->pId);
  free(this);
#ifdef SHOW_DESTORY
  printf("UnpackingThread destory once\n");
#endif
}