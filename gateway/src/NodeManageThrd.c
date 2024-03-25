#include "NodeManageThrd.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "LoraGateway.h"
#include "Node.h"
#include "NodeManager.h"

NodeManageThread* spNewNodeManageThread() {
  NodeManageThread* this = (NodeManageThread*)malloc(sizeof(NodeManageThread));
  this->pId = (pthread_t*)malloc(sizeof(pthread_t));
  this->vpStartRoutine = (void*)vNodeManageRun;
  this->vpArg = NULL;
  this->vCreate = vCreateNodeManageThread;
  return this;
}
void vNodeManageRun() {
  printf("NodeManageRun start\n");
  Node* spNode = NULL;
  while (loraGateway->bCheckNoSignal()) {
    sleep(1);
    // 先判断是否有下线的 | 这个地方，考虑每次单独取时间有必要？
    int iNowTime = loraGateway->iGetSysTime();
    for (int i = 0; i < GroupNum; ++i) {
      for (int j = 0; j < PerGroupNodeNum; ++j) {
        spNode = &nodeManager->saNodeArr[i][j];
        // 如果节点30s未上传数据，判定下线
        if (spNode->bOnline &&
            ((iNowTime + 60000 - spNode->iBackTime) % 60000 > 30000)) {
          nodeManager->iOnlineNodeNum--;
          printf("%03d组 %d号 节点%08d 已下线\n", i, j, spNode->iAddr);
          spNode->bOnline = false;
          spNode->iAddr = 0;
          spNode->iBackTime = 0;
          // 这里向上告知 do something
        }
      }
    }
    // 回发ACK
    nodeManager->vACK();
    
    // 然后写回本地
    FILE* fpRewrite = fopen("Grouping.txt", "r+");
    fseek(fpRewrite, 0, SEEK_SET);
    for (int i = 0; i < GroupNum; ++i) {
      for (int j = 0; j < PerGroupNodeNum - 1; ++j) {
        spNode = &nodeManager->saNodeArr[i][j];
        fprintf(fpRewrite, "%8x %5d ", spNode->iAddr, spNode->iBackTime);
      }
      spNode = &nodeManager->saNodeArr[i][PerGroupNodeNum - 1];
      fprintf(fpRewrite, "%8x %5d\n", spNode->iAddr, spNode->iBackTime);
    }
    fclose(fpRewrite);
  }
}
void vCreateNodeManageThread(NodeManageThread* this) {
  if (0 != pthread_create(this->pId, NULL, this->vpStartRoutine, this->vpArg)) {
    printf("Create NodeManageThread error!\n");
  } else {
#ifdef SHOW_SUCCESS
    printf("Create NodeManageThread successfully\n");
#endif
  }
  return;
};

void vDestoryNodeManageThread(NodeManageThread* this) {
  pthread_join(*(this->pId), NULL);
#ifdef SHOW_DESTORY
  printf("Destory\n");
#endif
  free(this->pId);
  free(this);
}