#include "UnpackingOperator.h"

#include <stdio.h>
#include <string.h>

#include "LoraGateway.h"
#include "LoraServer.h"
#include "stdlib.h"
UnpackingOperator *spNewUnpackingOperator() {
  UnpackingOperator *this =
      (UnpackingOperator *)malloc(sizeof(UnpackingOperator));
  this->iOnlineNodeNum = 0;
  this->iMaxGroupNum = -1;
  this->vInit = vInit;
  this->vOutPutNodeInfo = vOutPutNodeInfo;
  this->vDo = vDo;
  return this;
}
void vFunTypeO(RxData *sData) {}
// 温度数据包处理函数
void vFunTypeT(RxData *sData) {}
// 分组请求包处理函数
void vFunTypeS(RxData *sData, UnpackingOperator *this) {
  // 标记是否分配
  bool bAllocate = false;
  // 赋值指针
  Node *spNode = NULL;
  // 数据包的节点地址 字符串版
  char caAddr[5] = "temp"; // 给caAddr[4]赋值'\0'
  strncpy(caAddr, sData->payload + 6, 4);
  // 数据包的节点地址 整形版
  int iAddr = caAddr[0] | caAddr[1] << 8 | caAddr[2] << 16 | caAddr[3] << 24;
  int iGroupId = -1, iIndexId = -1;
#ifdef USE_ADJUST_SF
  if (sData->rssic > MIN_RSSI && sData->rssic < MAX_RSSI) {
  }
#endif
  // 先查找该地址是否已分配，若已分配，返回登记的[组号+下标]即可
  for (int i = 0; i < GroupNum && i <= this->iMaxGroupNum && !bAllocate; ++i) {
    for (int j = 0; j < PerGroupNodeNum; ++j) {
      spNode = &this->saNodeArr[i][j];
      if (true == spNode->bOnline && spNode->iAddr == iAddr) {
        bAllocate = true;  // 分配过了！
        iGroupId = i;
        iIndexId = j;
        break;
      }
    }
  }
  if (!bAllocate) {
    // 若未进行分组，则找一个空位给它 | 这个地方i 和 j的顺序 哪个好呢？
    for (int i = 0; i < GroupNum && !bAllocate; ++i) {
      for (int j = 0; j < PerGroupNodeNum; ++j) {
        spNode = &this->saNodeArr[i][j];
        if (false == spNode->bOnline) {
          bAllocate = true;  // 分配成功！
          spNode->bOnline = true;
          spNode->iAddr = iAddr;
          iGroupId = i;
          iIndexId = j;
          break;
        }
      }
    }
  }
  if (bAllocate) {
    spNode->iBackTime = loraGateway->iGetSysTime();
    printf("address:%08x backtime:%05d group:%03d index:%d\n", iAddr,
           spNode->iBackTime, iGroupId, iIndexId);
    char caDatas[100];
    // 帧格式
    sprintf(caDatas, "start%s-%05d-%03d%d\r\n", caAddr, spNode->iBackTime, iGroupId, iIndexId);
    printf("%s", caDatas);
    loraGateway->v1302Send(loraGateway, caDatas);
  } else {
    printf("分配失败\n");
  }
}
// 错误包处理函数
void vFunTypeE() {
  // 错误包掠过就是了……还处理个锤子
  // 或者想记录一下错误率倒是无可厚非
#ifdef SHOW_MORE
  printf("收到一条错误数据\n");
#endif
}
void vDo(Packet *pPacket, UnpackingOperator *this) {
  for (int i = 0; i < pPacket->iLen; ++i) {
    switch (pPacket->cType[i]) {
      // 错误包
      case 'E':
        vFunTypeE();
        break;
      // 分组请求包
      case 'S':
        vFunTypeS(&(pPacket->rxPkt[i]), this);
        break;
      // 温度数据包
      case 'T':
        vFunTypeT(&(pPacket->rxPkt[i]));
        break;
      // 其他数据包
      case 'O':
        vFunTypeO(&(pPacket->rxPkt[i]));
    }
  }
}
void vOutPutNodeInfo(UnpackingOperator *this) {
  Node *spNode = NULL;
  for (int i = 0; i < GroupNum; ++i) {
    for (int j = 0; j < PerGroupNodeNum; ++j) {
      spNode = &this->saNodeArr[i][j];
      printf("%8x %5d ", spNode->iAddr, spNode->iBackTime);
    }
    printf("|%3d\n", i);
  }
  printf("OnlineNodeNum:%d\n", this->iOnlineNodeNum);
  printf("MaxGroupNum:%d\n", this->iMaxGroupNum);
}
void vInit(UnpackingOperator *this) {
  // Control控制是否清除节点记录 y表示清除 其他不清除
  if (Control[0] && !strcmp(Control[0], "y")) {
    FILE *fpClear = fopen("Grouping.txt", "w+");
    char buf[20 * PerGroupNodeNum + 1];
    sprintf(buf, "%8x %5x %8x %5x %8x %5x %8x %5x", 0, 0, 0, 0, 0, 0, 0, 0);
    for (int i = 0; i < GroupNum; ++i) {
      fprintf(fpClear, "%s\n", buf);
    }
    memset(this->saNodeArr, 0, sizeof(this->saNodeArr));
    fclose(fpClear);
  }
  FILE *fpRead = fopen("Grouping.txt", "r+");
  fseek(fpRead, 0, SEEK_SET);
  for (int i = 0; i < GroupNum; ++i) {
    for (int j = 0; j < PerGroupNodeNum; ++j) {
      Node *spNode = &this->saNodeArr[i][j];
      fscanf(fpRead, "%x%d", &spNode->iAddr, &spNode->iBackTime);
      // 记录未分配的节点数量 和 最大Id，用于后续遍历
      if (0 != spNode->iAddr) {
        spNode->bOnline = true;
        this->iOnlineNodeNum++;
        this->iMaxGroupNum = i;
      } else {
        spNode->bOnline = false;
      }
    }
  }
  fclose(fpRead);
}