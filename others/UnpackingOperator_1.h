#ifndef __UNPACKING_OPERATOR_H
#define __UNPACKING_OPERATOR_H

#include "Packet.h"
#include "Node.h"
#define GroupNum 10        // 组数
#define PerGroupNodeNum 4  // 每组包含几个节点

typedef struct UnpackingOperator UnpackingOperator;
struct UnpackingOperator {
  // 在线节点数量      最大组号
  int iOnlineNodeNum, iMaxGroupNum;
  Node saNodeArr[GroupNum][PerGroupNodeNum];
  // 1表示新建，0表示读取旧记录
  void (*vInit)(UnpackingOperator* this);
  void (*vOutPutNodeInfo)(UnpackingOperator* this);
  void (*vDo)(Packet*, UnpackingOperator *this);
};
// 其他数据包处理函数
void vFunTypeO(RxData* sData);
// 温度数据包处理函数
void vFunTypeT(RxData* sData);
// 分组请求包处理函数
void vFunTypeS(RxData* sData);
// 错误包处理函数
void vFunTypeE();
// 输出节点信息
void vDo(Packet* pPacket);
void vOutPutNodeInfo(UnpackingOperator* this);
void vInit(UnpackingOperator* this);  // 1表示新建，0表示读取旧记录
// 处理函数
UnpackingOperator* spNewUnpackingOperator();
void vDestoryUnpackingOperator(UnpackingOperator* this);
extern UnpackingOperator* unpackingOperator;
#endif