#ifndef __LORA_SERVER_H
#define __LORA_SERVER_H

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
// #define SHOW_DESTORY   // 不显示析构信息请注释该行
// #define SHOW_SUCCESS   // 不显示成功信息请注释该行
// #define SHOW_MORE      // 不显示更多信息请注释该行
#define USE_ADJUST_SF  // 不使用SF调整请注释该行
// 丢包测试（需要节点连续发送编号）
// #define TEST

// 最大最小RSSI
#ifdef USE_ADJUST_SF
#define MAX_RSSI 0
#define MIN_RSSI -100
#endif

// 缓冲器最大长度
#define BUFFERMAX 64
// 控制信号集合
#define CONTROL_NUM 2
extern char *Control[CONTROL_NUM];
/* Control_0控制是否清除节点记录 y表示清除 其他不清除
 *
 *
 */
#ifdef TEST
extern int iSendTimesA, iReceTimesA;
extern int iSendTimesB, iReceTimesB;
extern int iSendTimesC, iReceTimesC;
extern int iSendTimesD, iReceTimesD;
#endif

#endif