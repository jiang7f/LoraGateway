#include "UnpackingOperator.h"

#include <stdio.h>
#include <string.h>

#include "LoraGateway.h"
#include "LoraServer.h"
#include "NodeManager.h"
#include "stdlib.h"
UnpackingOperator *spNewUnpackingOperator() {
  UnpackingOperator *this = (UnpackingOperator *)malloc(sizeof(UnpackingOperator));
  this->vUnpackingOperatorInit = vUnpackingOperatorInit;
  this->vDo = vDo;
  return this;
}
void vDo(Packet *pPacket) {
  for (int i = 0; i < pPacket->iLen; ++i) {
    switch (pPacket->cType[i]) {
      // 分组请求包
      case 'S':
        vFunTypeS(&(pPacket->rxPkt[i]));
        break;
      // 温湿度数据包
      case 'M':
        vFunTypeM(&(pPacket->rxPkt[i]));
        break;
      // 错误包
      case 'E':
        vFunTypeE();
        break;
      // 其他数据包
      case 'O':
        vFunTypeO(&(pPacket->rxPkt[i]));
        break;
    }
  }
}
void vFunTypeO(RxData *sData) {}
// 温度数据包处理函数
void vFunTypeM(RxData *sData) {
  // 数据包的节点地址 字符串版
  char caAddr[9] = "00000000";  // 给caAddr[9]赋值'\0'
  strncpy(caAddr, sData->payload, 8);
  // 数据包的节点地址 整形版
  int iAddr= 0;
  sscanf(caAddr, "%08X", &iAddr);
  // 组号 序号：待用
  int iGroupId = -1, iIndexId = -1;
  bool bAllocate = nodeManager->bIsAllocated(iAddr, &iGroupId, &iIndexId);
  if (bAllocate) {
    char pubMessage[30];
    int id, tp, hm;
    sscanf(sData->payload, "%08X_tphm_%d_%d", &id, &tp, &hm);
    sprintf(pubMessage, "%08X_%d_%d", id, tp, hm);
    // // 帧格式
    // sprintf(caDatas, "", );
    // printf("%s", caDatas);
    // loraGateway->v1302Send(loraGateway, caDatas);
    loraGateway->vMQTTPublish("message", pubMessage);
  }
}
// 分组请求包处理函数
void vFunTypeS(RxData *sData) {
  // 标记是否分配
  bool bAllocate = false;
  // 赋值指针
  Node *spNode = NULL;
  // 数据包的节点地址 字符串版
  char caAddr[9] = "00000000";  // 给caAddr[9]赋值'\0'
  strncpy(caAddr, sData->payload + 6, 8);
  // 数据包的节点地址 整形版
  int iAddr= 0;
  sscanf(caAddr, "%08X", &iAddr);
  nodeManager->vAllocate(iAddr);
}
// 错误包处理函数
void vFunTypeE() {
  // 错误包掠过就是了……还处理个锤子
  // 或者想记录一下错误率倒是无可厚非
#ifdef SHOW_MORE
  printf("收到一条错误数据\n");
#endif
}

void vUnpackingOperatorInit() {}
void vDestoryUnpackingOperator(UnpackingOperator *this) { free(this); }