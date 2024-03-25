#ifndef __NODE_MANAGER_H
#define __NODE_MANAGER_H


#include "Node.h"

#define GroupNum 5        // 组数
#define PerGroupNodeNum 8  // 每组包含几个节点

typedef struct NodeManager NodeManager;
struct NodeManager {

  // 在线节点数量      最大组号
  int iOnlineNodeNum, iMaxGroupNum;
  Node saNodeArr[GroupNum][PerGroupNodeNum];
  // 1表示新建，0表示读取旧记录
  void (*vOutPutNodeInfo)();
  void (*vNodeManagerInit)();
  void (*vAllocate)(int);
  bool (*bIsAllocated)(int iAddr, int *iGroupId, int *iIndexId);
  void (*vACK)();
};
void vACK();
// 判断是否已分配
bool bIsAllocated(int iAddr, int *iGroupId, int *iIndexId);
// 根据int类型的iAddr分组
void vAllocate(int iAddr);
// 异步IO接收 接收部分的代码
void vAsyncIoInit();
// 分组函数
void vNodeManagerInit();
void vOutPutNodeInfo();
NodeManager* spNewNodeManager();
void vDestoryNodeManager(NodeManager* this);
extern NodeManager* nodeManager;
#endif