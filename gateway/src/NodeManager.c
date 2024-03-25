#define _GNU_SOURCE  // 在源文件开头定义_GNU_SOURCE 宏
#include "NodeManager.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "LoraGateway.h"
#include "LoraServer.h"
#include "Node.h"
NodeManager* spNewNodeManager() {
  NodeManager* this = (NodeManager*)malloc(sizeof(NodeManager));
  this->iOnlineNodeNum = 0;
  this->iMaxGroupNum = -1;
  this->vNodeManagerInit = vNodeManagerInit;
  this->vOutPutNodeInfo = vOutPutNodeInfo;
  this->vAllocate = vAllocate;
  this->bIsAllocated = bIsAllocated;
  this->vACK = vACK;
  return this;
}
void vACK() {
  // 这里可以用状压来节省通信量，ans += 1 << j, 但是为了好看，直接用字符串了
  char sACK[PerGroupNodeNum + 1];
  for (int i = 0; i < PerGroupNodeNum + 1; ++i) {
    sACK[i] = '_';
  }
  sACK[PerGroupNodeNum] = '\0';
  //
  int iNowTime = loraGateway->iGetSysTime();
  // 这里有个偏移，举个例子，0组，会往后延迟1秒，
  // 在[1, 2)的时间发送，所以0组在2s返回ACK, 即时间往前偏移2为组号
  int iGroupId = (iNowTime / 1000 + GroupNum -  2) % GroupNum;
  // 赋值指针
  Node* spNode = NULL;
  bool bOneMoreOnline = false;
  for (int j = 0; j < PerGroupNodeNum; ++j) {
    spNode = &nodeManager->saNodeArr[iGroupId][j];
    if (true == spNode->bOnline) {
      bOneMoreOnline = true;
      if ((iNowTime + 60000 - spNode->iBackTime) % 60000 < 2000) {
        sACK[j] = '1';
      } else {
        sACK[j] = '0';
      }
    }
  }
  if (bOneMoreOnline) {
    printf("%d_ACK:%s\n", iGroupId, sACK);
    char caDatas[100];
    sprintf(caDatas, "%02d:%s\r\n", iGroupId, sACK);
    loraGateway->v1302Send(loraGateway, caDatas);
  }
}
bool bIsAllocated(int iAddr, int* iGroupId, int* iIndexId) {
  // 标记是否分配
  bool bAllocate = false;
  // 赋值指针
  Node* spNode = NULL;
  // 先查找该地址是否已分配，若已分配，返回登记的[组号+下标]即可
  for (int i = 0; i < GroupNum && i <= nodeManager->iMaxGroupNum && !bAllocate;
       ++i) {
    for (int j = 0; j < PerGroupNodeNum; ++j) {
      spNode = &nodeManager->saNodeArr[i][j];
      if (true == spNode->bOnline && spNode->iAddr == iAddr) {
        bAllocate = true;  // 分配过了！
        *iGroupId = i;
        *iIndexId = j;
        break;
      }
    }
  }
  if (bAllocate) {
    spNode->iBackTime = loraGateway->iGetSysTime();
  }
  return bAllocate;
}
void vAllocate(int iAddr) {
  // 标记是否分配
  bool bAllocate = false;
  // 赋值指针
  Node* spNode = NULL;
  // 数据包的节点地址 字符串版
  int iGroupId = -1, iIndexId = -1;
  // 先查找该地址是否已分配，若已分配，返回登记的[组号+下标]即可
  for (int i = 0; i < GroupNum && i <= nodeManager->iMaxGroupNum && !bAllocate;
       ++i) {
    for (int j = 0; j < PerGroupNodeNum; ++j) {
      spNode = &nodeManager->saNodeArr[i][j];
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
        spNode = &nodeManager->saNodeArr[i][j];
        if (false == spNode->bOnline) {
          bAllocate = true;  // 分配成功！
          spNode->bOnline = true;
          spNode->iAddr = iAddr;
          iGroupId = i;
          iIndexId = j;
          if (i > nodeManager->iMaxGroupNum) {
            nodeManager->iMaxGroupNum = i;
          }
          break;
        }
      }
    }
  }
  if (bAllocate) {
    spNode->iBackTime = loraGateway->iGetSysTime();
    printf("address:%08X backtime:%05d group:%03d index:%d\n", iAddr,
           spNode->iBackTime, iGroupId, iIndexId);
    char caDatas[100];
    // 帧格式
    sprintf(caDatas, "start-%08X-%05d-%03d%d\r\n", iAddr, spNode->iBackTime,
            iGroupId, iIndexId);
    printf("%s", caDatas);
    loraGateway->v1302Send(loraGateway, caDatas);
  } else {
    printf("分配失败\n");
  }
}
void vOutPutNodeInfo() {
  Node* spNode = NULL;
  for (int i = 0; i < GroupNum; ++i) {
    for (int j = 0; j < PerGroupNodeNum; ++j) {
      spNode = &nodeManager->saNodeArr[i][j];
      printf("%8x %5d ", spNode->iAddr, spNode->iBackTime);
    }
    printf("|%3d\n", i);
  }
  printf("OnlineNodeNum:%d\n", nodeManager->iOnlineNodeNum);
  printf("MaxGroupNum:%d\n", nodeManager->iMaxGroupNum);
}
void vNodeManagerInit() {
  // Control控制是否清除节点记录 y表示清除 其他不清除
  if (Control[0] && !strcmp(Control[0], "y")) {
    FILE* fpClear = fopen("Grouping.txt", "w+");
    char buf[20 * PerGroupNodeNum + 1];
    sprintf(buf, "%8x %5x %8x %5x %8x %5x %8x %5x", 0, 0, 0, 0, 0, 0, 0, 0);
    for (int i = 0; i < GroupNum; ++i) {
      fprintf(fpClear, "%s\n", buf);
    }
    memset(nodeManager->saNodeArr, 0, sizeof(nodeManager->saNodeArr));
    fclose(fpClear);
  }
  FILE* fpRead = fopen("Grouping.txt", "r+");
  fseek(fpRead, 0, SEEK_SET);
  for (int i = 0; i < GroupNum; ++i) {
    for (int j = 0; j < PerGroupNodeNum; ++j) {
      Node* spNode = &nodeManager->saNodeArr[i][j];
      fscanf(fpRead, "%x%d", &spNode->iAddr, &spNode->iBackTime);
      // 记录未分配的节点数量 和 最大Id，用于后续遍历
      if (0 != spNode->iAddr) {
        spNode->bOnline = true;
        nodeManager->iOnlineNodeNum++;
        nodeManager->iMaxGroupNum = i;
      } else {
        spNode->bOnline = false;
      }
    }
  }
  fclose(fpRead);
}
void vDestoryNodeManager(NodeManager* this) { free(this); }