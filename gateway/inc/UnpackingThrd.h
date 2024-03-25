#ifndef __UNPACKING_THRD_H
#define __UNPACKING_THRD_H

#include <pthread.h>

#include "LoraServer.h"
#include "Packet.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
// ******************** Thread of Unpacing ********************
typedef struct UnpackingThread UnpackingThread;
struct UnpackingThread {
  pthread_t* pId;
  void* (*vpStartRoutine)(void*);
  void* argv[2];
  void* vpArg;
  void (*vCreate)(UnpackingThread*);
};

typedef struct RetNum RetNum;
struct RetNum {
  int groupNum, numb;
};

void vUnpackingRun(void* argv[]);

// 线程2构造函数
UnpackingThread* spNewUnpackingThread(Packet* packet);
// 线程2初始化函数
void vCreateUnpackingThread(UnpackingThread* this);
// 线程2销毁函数
void vDestoryUnpackingThread(UnpackingThread* this);

#endif