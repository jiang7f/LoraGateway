#ifndef __LORA_GATEWAY_H
#define __LORA_GATEWAY_H

#include <netinet/in.h>
#include <stdbool.h>

#include "MQTTClient.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
// #include <time.h>
// #include <unistd.h>
// #include <signal.h>
#include <termios.h>

#include "LoraServer.h"
#include "Packet.h"
#define LINUXDEV_PATH_DEFAULT "/dev/spidev0.0"
#define IP "192.168.85.10"
#define PORT 4001

#define DEVICE "/dev/device3"

// MQTT连接参数
#define MAX_CONNECT_TIME 2
#define RECONNECT_DELAY_SECONDS 3
#define MAX_RECONNECT_ATTEMPTS 20
#define ADDRESS "tcp://we102f7e.cn-hangzhou.emqx.cloud:15806"
#define USERNAME "raspberrypi_jqf"
#define PASSWORD "123"
#define CLIENTID "raspberry"
#define QOS 0
#define SUBSCRIBE_TOPIC "control"
#define TIMEOUT 10000L



typedef struct LoraGateway LoraGateway;
struct LoraGateway {
  bool quit_sig, exit_sig;  // 退出信号控制
  struct sockaddr_in caddr;
  int csize, skfdd;
  TxData txData;
  struct termios sOldConfig;
  void (*v1302SendInit)();
  void (*vTcpServerInit)(LoraGateway* this);
  void (*vMQTTConnectInit)();
  bool (*bCheckNoSignal)();
  int (*iGetSysTime)();
  void (*vWaitMs)(long ms);
  int (*iRxFromLora)(DataArray aDataArray);
  void (*v1302Send)(LoraGateway* this, char* spMessage);
  void (*vMQTTPublish)(char *topic, char *payload);
};
// MQTT连接
void vMQTTConnectInit();
int on_message(void* context, char* topicName, int topicLen, MQTTClient_message* message);
// MQTT发布
void vMQTTPublish(char *topic, char *payload);
// 1302发送
void v1302Send(LoraGateway* this, char* spMessage);
// 从Lora缓冲器接收
int iRxFromLora(DataArray aDataArray);
void vWaitMs(long ms);
// 返回时间
int iGetSysTime();
// 公用LoraGateway
// 查询是否给了退出信号
bool bCheckNoSignal();
void vTcpServerInit(LoraGateway* this);
/*
  初始化网关函数
  配置if信道频率偏移，rf中心频点等参数
*/
void v1302SendInit(LoraGateway* this);
LoraGateway* spNewLoraGateway();
// 始化函数
void vInitLoraGateway(LoraGateway* this);
// 绑定信号中断控制函数 静态函数，定义在.c文件中
// static void sig_handler(int sigio);
// 析构函数
void vDestoryLoraGateway(LoraGateway* this);
extern int iWaitMs;
extern LoraGateway* loraGateway;
#endif