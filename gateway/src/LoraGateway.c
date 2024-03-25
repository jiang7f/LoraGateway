#include "LoraGateway.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "LoraGateway.h"
#include "LoraServer.h"
#include "Node.h"
#include "NodeManager.h"
#include "loragw_hal.h"
// Loragateway初始化
static void sig_handler(int sigio);
LoraGateway* spNewLoraGateway() {
  LoraGateway* this = (LoraGateway*)malloc(sizeof(LoraGateway));
  this->exit_sig = false;
  this->quit_sig = false;
  this->v1302SendInit = v1302SendInit;
  this->vTcpServerInit = vTcpServerInit;
  this->vMQTTConnectInit = vMQTTConnectInit;
  this->bCheckNoSignal = bCheckNoSignal;
  this->iGetSysTime = iGetSysTime;
  this->vWaitMs = vWaitMs;
  this->iRxFromLora = iRxFromLora;
  this->v1302Send = v1302Send;
  this->v1302Send = v1302Send;
  this->vMQTTPublish = vMQTTPublish;
  // 绑定信号中断函数
  if (signal(SIGQUIT, sig_handler) == SIG_ERR) {
    perror("signal failed");  // 注册SIGQUIT信号处理函数失败
    exit(EXIT_FAILURE);
  }
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    perror("signal failed");  // 注册SIGINT信号处理函数失败
    exit(EXIT_FAILURE);
  }
  if (signal(SIGTERM, sig_handler) == SIG_ERR) {
    perror("signal failed");  // 注册SIGTERM信号处理函数失败
    exit(EXIT_FAILURE);
  }
  return this;
}
// MQTT连接初始化
MQTTClient client;  // 定义在外面喽，没规范写，先这样
void vMQTTConnectInit() {
  int rc;
  MQTTClient_create(&client, ADDRESS, CLIENTID, 0, NULL);
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.username = USERNAME;
  conn_opts.password = PASSWORD;
  // conn_opts.connectTimeout = 5; 这么设置没用
  MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);
  int reconnect_attempts = 0;  // 连接尝试次数
  /*
    一次连接，失败后结束
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
      printf("Failed to connect, return code %d\n", rc);
      exit(-1);
    } else {
      printf("Connected to MQTT Broker!\n");
    }
  */
  do {
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
      printf("Failed to connect, return code %d\n", rc);
      // 等待一定时间后尝试重新连接
      printf("Reconnecting in %d seconds...\n", RECONNECT_DELAY_SECONDS);
      sleep(RECONNECT_DELAY_SECONDS);
      // 增加重连尝试次数
      reconnect_attempts++;
    } else {
      printf("Connected to MQTT Broker!\n");
      MQTTClient_subscribe(client, "control", QOS);
    }
  } while (loraGateway->bCheckNoSignal() && rc != MQTTCLIENT_SUCCESS &&
           reconnect_attempts < MAX_RECONNECT_ATTEMPTS);

  // MQTTClient_subscribe(client, SUBSCRIBE_TOPIC, QOS);
}
// MQTT发布消息
void vMQTTPublish(char* topic, char* payload) {
  MQTTClient_message message = MQTTClient_message_initializer;
  message.payload = payload;
  message.payloadlen = strlen(payload);
  message.qos = QOS;
  message.retained = 0;
  MQTTClient_deliveryToken token;
  MQTTClient_publishMessage(client, topic, &message, &token);
  MQTTClient_waitForCompletion(client, token, TIMEOUT);
  printf("Send '%s' to topic '%s' \n", payload, topic);
}
void vMQTTControl(char* sData) {
  // 标记是否分配
  bool bAllocate = false;
  // 赋值指针
  Node* spNode = NULL;
  // 数据包的节点地址 字符串版
  char caAddr[9] = "00000000";  // 给caAddr[9]赋值'\0'
  if (!strncmp(sData, "start", 5)) {
    strncpy(caAddr, sData + 6, 8);
    // 数据包的节点地址 整形版
    int iAddr = 0;
    sscanf(caAddr, "%08X", &iAddr);
    nodeManager->vAllocate(iAddr);
  } else {
    char caDatas[100];
    // 帧格式
    sprintf(caDatas, "%s\r\n", sData);
    loraGateway->v1302Send(loraGateway, caDatas);
  }
}

// MQTT接收消息子函数
int on_message(void* context, char* topicName, int topicLen,
               MQTTClient_message* message) {
  char* payload = message->payload;
  printf("Received '%s' from '%s' topic \n", payload, topicName);
  vMQTTControl(payload);
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);
  return 1;
}
void v1302Send(LoraGateway* this, char* spMessage) {
  this->txData.size = strlen(spMessage);
  memset(this->txData.payload, 0, sizeof(this->txData.payload));
  strcpy((char*)this->txData.payload, spMessage);
  /* connect, configure and start the LoRa concentrator */
  int x;
  uint8_t tx_status;
  x = lgw_send(&this->txData);
  /* wait for packet to finish sending */
  do {
    vWaitMs(5);
    /* get TX status */
    lgw_status(this->txData.rf_chain, TX_STATUS, &tx_status);
  } while ((tx_status != TX_FREE));
  if (x != 0) {
    printf("ERROR: failed to send packet\n");
    return;
  }
}
// 从Lora缓冲器接收
int iRxFromLora(DataArray aDataArray) {
  memset(aDataArray, 0, sizeof(DataArray));
  return lgw_receive(BUFFERMAX, aDataArray);
}
// 毫秒延迟
void vWaitMs(long ms) { usleep(ms * 1000); }
// 获得系统时间 格式是 (1000*s+ms)
int iGetSysTime() {
  struct timeb t;
  ftime(&t);
  return 1000 * (t.time % 60) + t.millitm;
}
// 信号中断函数
static void sig_handler(int sigio) {
  switch (sigio) {
    case SIGQUIT:
      loraGateway->quit_sig = 1;
      break;
    case SIGINT:
    case SIGTERM:
      loraGateway->exit_sig = 1;
      break;
    default:
      printf("Unhandled signald %d\n", sigio);  // 打印未处理的信号
      break;
  }
}
// 检查退出信号
bool bCheckNoSignal() {
  return ((loraGateway->exit_sig != 1) && (loraGateway->quit_sig != 1));
}

void vTcpServerInit(LoraGateway* this) {
  int skfd = -1, ret = -1;
  skfd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == skfd) {
    printf("socket failed\n");
  }
  int on = 1;
  setsockopt(skfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;             // 设置tcp协议族
  addr.sin_port = htons(PORT);           // 设置端口号
  addr.sin_addr.s_addr = inet_addr(IP);  // 设置ip地址
  ret = bind(skfd, (struct sockaddr*)&addr, sizeof(addr));
  if (-1 == ret) {
    printf("bind failed\n");
  }
  ret = listen(skfd, 3);
  if (-1 == ret) {
    printf("listen failed\n");
  }
  this->skfdd = skfd;
}
void v1302SendInit(LoraGateway* this) {
  uint32_t ft, fa, fb;
  struct lgw_pkt_tx_s* pkt = (struct lgw_pkt_tx_s*)&this->txData;
  // struct sigaction sigact; /* SIGQUIT&SIGINT&SIGTERM signal handling */
  struct lgw_conf_board_s boardconf;
  uint8_t clocksource = 0;
  const char spidev_path_default[] = LINUXDEV_PATH_DEFAULT;
  const char* spidev_path = spidev_path_default;
  struct lgw_conf_rxrf_s rfconf;
  struct lgw_conf_rxif_s ifconf;
  uint8_t rf_chain = 0;
  struct lgw_tx_gain_lut_s txlut; /* TX gain table */
  struct lgw_rssi_tcomp_s rssi_tcomps;
  int i, flag;

  const int32_t channel_if_mode0[9] = {
      -400000, -200000, 0,       200000, 200000,
      0,       -400000, -200000, 400000 /* lora service */
  };

  const uint8_t channel_rfchain_mode0[9] = {0, 0, 0, 0, 1, 1, 1, 1, 1};

  /* Initialize TX gain LUT */
  txlut.size = 0;
  memset(txlut.lut, 0, sizeof txlut.lut);
  /* .5 Hz offset to get rounding instead of truncating */
  ft = (uint32_t)((506.5 * 1e6) + 0.5);
  fa = (uint32_t)((475.9 * 1e6) + 0.5);
  fb = (uint32_t)((fa + 800000 + 0.5));

  /* Configure the gateway */
  boardconf.lorawan_public = false;
  boardconf.clksrc = clocksource;
  boardconf.full_duplex = false;
  strncpy(boardconf.spidev_path, spidev_path, sizeof boardconf.spidev_path);
  boardconf.spidev_path[sizeof boardconf.spidev_path - 1] =
      '\0'; /* ensure string termination */
  if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
    printf("ERROR: failed to configure board\n");
    // return EXIT_FAILURE;
    return;
  }

  rssi_tcomps.coeff_a = 0;
  rssi_tcomps.coeff_b = 0;
  rssi_tcomps.coeff_c = 20.41;
  rssi_tcomps.coeff_d = 2162.56;
  rssi_tcomps.coeff_e = 0;

  memset(&rfconf, 0, sizeof rfconf);
  /* rf chain 0 needs to be enabled for calibration to work on sx1257 */
  rfconf.enable = true;
  rfconf.freq_hz = fa;
  rfconf.type = LGW_RADIO_TYPE_SX1250;
  rfconf.rssi_offset = -207.0;
  rfconf.tx_enable = true;
  rfconf.rssi_tcomp = rssi_tcomps;
  rfconf.single_input_mode = false;
  if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS) {
    printf("ERROR: failed to configure rxrf 0\n");
    // return EXIT_FAILURE;
    return;
  }

  memset(&rfconf, 0, sizeof rfconf);
  rfconf.enable = true;
  rfconf.freq_hz = fb;
  rfconf.type = LGW_RADIO_TYPE_SX1250;
  rfconf.rssi_offset = -207.0;
  rfconf.tx_enable = false;
  rfconf.rssi_tcomp = rssi_tcomps;
  rfconf.single_input_mode = false;
  if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS) {
    printf("ERROR: failed to configure rxrf 1\n");
    // return EXIT_FAILURE;
    return;
  }

  /* set configuration for LoRa multi-SF channels (bandwidth cannot be set) */
  memset(&ifconf, 0, sizeof(ifconf));
  for (i = 0; i < 8; i++) {
    ifconf.enable = true;
    ifconf.rf_chain = channel_rfchain_mode0[i];
    ifconf.freq_hz = channel_if_mode0[i];
    ifconf.datarate = DR_LORA_SF7;
    if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS) {
      printf("ERROR: failed to configure rxif %d\n", i);
      // return EXIT_FAILURE;
      return;
    }
  }

  /* set configuration for LoRa Service channel */
  memset(&ifconf, 0, sizeof(ifconf));
  ifconf.enable = true;
  ifconf.rf_chain = channel_rfchain_mode0[i];
  ifconf.freq_hz = channel_if_mode0[i];
  ifconf.datarate = DR_LORA_SF7;
  ifconf.bandwidth = BW_125KHZ;
  if (lgw_rxif_setconf(8, &ifconf) != LGW_HAL_SUCCESS) {
    printf("ERROR: failed to configure rxif for LoRa service channel\n");
    // return EXIT_FAILURE;
    return;
  }

  if (txlut.size > 0) {
    if (lgw_txgain_setconf(rf_chain, &txlut) != LGW_HAL_SUCCESS) {
      printf("ERROR: failed to configure txgain lut\n");
      // return EXIT_FAILURE;
      return;
    }
  }

  memset(pkt, 0, sizeof this->txData);
  pkt->rf_chain = 0;
  pkt->freq_hz = ft;
  pkt->rf_power = (int8_t)20;
  pkt->tx_mode = IMMEDIATE;
  pkt->modulation = MOD_LORA;
  pkt->coderate = CR_LORA_4_5;
  pkt->no_crc = false;
  pkt->invert_pol = false;
  pkt->preamble = 8;
  pkt->no_header = false;
  pkt->bandwidth = BW_125KHZ;
  pkt->datarate = 7;  // sf

  if (system("./reset_lgw.sh start") != 0) {
    printf("ERROR: failed to reset SX1302, check your reset_lgw.sh script\n");
    exit(EXIT_FAILURE);
  }
  /* connect, configure and start the LoRa concentrator */
  flag = lgw_start();
  if (flag != 0) {
    printf("ERROR: failed to start the gateway\n");
    // return EXIT_FAILURE;
    return;
  }
  printf("Waiting for packets...\n");
  return;
}
void vDestoryLoraGateway(LoraGateway* this) {
  MQTTClient_disconnect(client, TIMEOUT);
  MQTTClient_destroy(&client);
  free(this);
  // 好像还需要做什么……
}
