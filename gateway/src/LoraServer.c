#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "NodeManageThrd.h"
#include "LoraGateway.h"
#include "PacketReceiveThrd.h"
#include "NodeManager.h"
#include "UnpackingOperator.h"
/*
 * 命名规则：
 * 1 [类名]大驼峰法
 * 2 [变量名、方法名、函数名]小驼峰变形
 *   第一个字母表示值（返回值）类型，其余单词首字母大写
 *   s表示结构体 p表示指针 a数组 i整形
 * ---------------------------------------------
 * 说明:
 * 1 pId指的是变量叫Id,类型是指针b,不是进程Id
 * 2 对数据包的定义与库中略有区别,请按如下概念区分变量名
 *   终端节点发送过来的定义为RxData
 *   DataArray为长度16的RxData数组
 *   Data数组+数组长度 定义为Packet
 */
char* Control[CONTROL_NUM] = {NULL};
int iWaitMs = 200;
#ifdef TEST
int iSendTimesA = 0, iReceTimesA = 0;
int iSendTimesB = 0, iReceTimesB = 0;
int iSendTimesC = 0, iReceTimesC = 0;
int iSendTimesD = 0, iReceTimesD = 0;
#endif
LoraGateway* loraGateway;
NodeManager* nodeManager;
UnpackingOperator* unpackingOperator;

int main(int argc, char* argv[]) {
  // 赋值控制集合 argv[] => Control[]
  for (int i = 1; i < argc && i <= CONTROL_NUM; ++i) {
    Control[i - 1] = argv[i];
  }
  // gateway对象
  loraGateway = spNewLoraGateway();
  loraGateway->v1302SendInit();
  loraGateway->vMQTTConnectInit();
  // loraGateway->vTcpServerInit(loraGateway);
  // 解包员
  unpackingOperator = spNewUnpackingOperator();
  unpackingOperator->vUnpackingOperatorInit();
  // 节点管理员
  nodeManager = spNewNodeManager();
  nodeManager->vNodeManagerInit();  // 1表示新建，0表示读取旧记录
  // 展示当前节点数
  nodeManager->vOutPutNodeInfo();
  // 节点管理线程
  NodeManageThread* nodeManageThread = spNewNodeManageThread();
  nodeManageThread->vCreate(nodeManageThread);
  // 数据接收线程
  PacketReceiveThread* packetReceiveThread = spNewPacketReceiveThread();
  packetReceiveThread->vCreate(packetReceiveThread);

  // Thread2* p2 = sNewThread2();
  // p2->vCreate(p2);
  while (loraGateway->bCheckNoSignal()) {
    loraGateway->vWaitMs(1000);
  }
#ifdef TEST
  printf("\n节点发送间隔100ms,等待%dms时\n", iWaitMs);
  printf("A共发送%d次,收到%d次\n", iSendTimesA, iReceTimesA);
  printf("丢包率为:%2.2f%%\n",
         100.0 * (iSendTimesA - iReceTimesA) / iSendTimesA);
  printf("B共发送%d次,收到%d次\n", iSendTimesB, iReceTimesB);
  printf("丢包率为:%2.2f%%\n",
         100.0 * (iSendTimesB - iReceTimesB) / iSendTimesB);
  printf("C共发送%d次,收到%d次\n", iSendTimesC, iReceTimesC);
  printf("丢包率为:%2.2f%%\n",
         100.0 * (iSendTimesC - iReceTimesC) / iSendTimesC);
  printf("D共发送%d次,收到%d次\n", iSendTimesD, iReceTimesD);
  printf("丢包率为:%2.2f%%\n",
         100.0 * (iSendTimesD - iReceTimesD) / iSendTimesD);
#endif
  if (system("./reset_lgw.sh stop") != 0) {
    printf("ERROR: failed to stop SX1302, check your reset_lgw.sh script\n");
    exit(EXIT_FAILURE);
  }
  vDestoryPacketReceiveThread(packetReceiveThread);
  vDestoryUnpackingOperator(unpackingOperator);
  vDestoryLoraGateway(loraGateway);
  vDestoryNodeManager(nodeManager);
  return 0;
}
