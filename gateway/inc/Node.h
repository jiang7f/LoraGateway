#ifndef __NODE_H
#define __NODE_H
#include <stdbool.h>
#include <sys/timeb.h>
typedef struct Node Node;
struct Node {
  bool bOnline;   // 是否在线
  int iAddr;      // 表示地址
  int iBackTime;  // 上一次反馈时间 0 ~ 59999
};
#endif