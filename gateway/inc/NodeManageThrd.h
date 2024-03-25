#ifndef __NODE_MANAGE_THRD_H
#define __NODE_MANAGE_THRD_H

#include <pthread.h>
// ******************** Thread of Node Manage ********************
typedef struct NodeManageThread NodeManageThread;
struct NodeManageThread {
  pthread_t* pId;
  void* (*vpStartRoutine)(void*);
  void* vpArg;
  void (*vCreate)(NodeManageThread*);
};
void vNodeManageRun();
// 线程3构造函数
NodeManageThread* spNewNodeManageThread();
// 线程3初始化函数
void vCreateNodeManageThread(NodeManageThread* this);
// 线程3销毁函数
void vDestoryNodeManageThread(NodeManageThread* this);

#endif