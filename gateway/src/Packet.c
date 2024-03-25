#include "Packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Packet* spNewPacket(DataArray rxPct, int len) {
  Packet* this = (Packet*)malloc(sizeof(Packet));
  memset(this, 0, sizeof(Packet));
  this->iLen = len;
  char* u8pPayLoad;  // ===定义一个PayLoad指针，减少重复申请
  for (int i = 0; i < len; ++i) {
    u8pPayLoad = (char*)(rxPct[i].payload);  // ===
    this->rxPkt[i] = rxPct[i];
    if (!strncmp(u8pPayLoad, "start", 5)) {
      this->cType[i] = 'S';
    } else if (!strncmp(u8pPayLoad + 9, "tphm", 4)) {
      this->cType[i] = 'M';
    } else {
      this->cType[i] = 'O';
    }
#ifdef SHOW_MORE
    // int index = *(u8pPayLoad + 8);
    // printf("||Index:%d\n", index);
    if (this->cType[i] != 'O') {
      printf("||Message:%s\n", u8pPayLoad);
      printf("||Type:%c\n", this->cType[i]);
    }
#endif
  }
  // 待拓展
  this->cAccessMethod = 0;
  return this;
}
void vDestoryPacket(Packet* this) {
  free(this);
#ifdef SHOW_DESTORY
  printf("Packet destory once\n");
#endif
}