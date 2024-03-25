#ifndef __UNPACKING_OPERATOR_H
#define __UNPACKING_OPERATOR_H

#include "Node.h"
#include "Packet.h"

typedef struct UnpackingOperator UnpackingOperator;
struct UnpackingOperator {
  void (*vUnpackingOperatorInit)();
  void (*vDo)(Packet*);
};
// 分组请求包处理函数
void vFunTypeS(RxData* sData);
// 其他数据包处理函数
void vFunTypeO(RxData* sData);
// 温度数据包处理函数
void vFunTypeM(RxData* sData);
// 错误包处理函数
void vFunTypeE();
// 输出节点信息
void vDo(Packet* pPacket);
void vUnpackingOperatorInit();  // 1表示新建，0表示读取旧记录
// 处理函数
UnpackingOperator* spNewUnpackingOperator();
void vDestoryUnpackingOperator(UnpackingOperator* this);
extern UnpackingOperator* unpackingOperator;
#endif