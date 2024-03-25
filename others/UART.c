#define _GNU_SOURCE  // 在源文件开头定义_GNU_SOURCE 宏
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
static struct termios sOldConfig;  // 用于保存终端的配置参数
static int fd;                     // 串口终端对应的文件描述符
/**
** 串口初始化操作
** 参数 device 表示串口终端的设备节点
**/
static int uart_init(const char *device) {
  /* 打开串口终端 */
  fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (0 > fd) {
    fprintf(stderr, "open error: %s: %s\n", device, strerror(errno));
    return -1;
  }
  /* 获取串口当前的配置参数 */
  if (0 > tcgetattr(fd, &sOldConfig)) {
    fprintf(stderr, "tcgetattr error: %s\n", strerror(errno));
    close(fd);
    return -1;
  }
  return 0;
}
/**
** 串口配置
** 参数 cfg 指向一个 uart_cfg_t 结构体对象
**/
static int uart_cfg() {
  struct termios sNewConfig = {0};  // 将 sNewConfig 对象清零
  speed_t speed;
  /* 设置为原始模式 */
  cfmakeraw(&sNewConfig);
  /* 使能接收 */
  sNewConfig.c_cflag |= CREAD;
  /* 设置波特率 */
  speed = B9600;
  if (0 > cfsetspeed(&sNewConfig, speed)) {
    fprintf(stderr, "cfsetspeed error: %s\n", strerror(errno));
    return -1;
  }
  /* 设置数据位大小 */
  sNewConfig.c_cflag &= ~CSIZE;  // 将数据位相关的比特位清零
  sNewConfig.c_cflag |= CS8;
  /* 设置奇偶校验 */
  sNewConfig.c_cflag &= ~PARENB;
  sNewConfig.c_iflag &= ~INPCK;
  /* 设置停止位 */
  sNewConfig.c_cflag &= ~CSTOPB;
  /* 将 MIN 和 TIME 设置为 0 */
  sNewConfig.c_cc[VTIME] = 0;
  sNewConfig.c_cc[VMIN] = 0;
  /* 清空缓冲区 */
  if (0 > tcflush(fd, TCIOFLUSH)) {
    fprintf(stderr, "tcflush error: %s\n", strerror(errno));
    return -1;
  }
  /* 写入配置、使配置生效 */
  if (0 > tcsetattr(fd, TCSANOW, &sNewConfig)) {
    fprintf(stderr, "tcsetattr error: %s\n", strerror(errno));
    return -1;
  }
  /* 配置 OK 退出 */
  return 0;
}
/**
** 信号处理函数，当串口有数据可读时，会跳转到该函数执行
**/
static void io_handler(int sig, siginfo_t *info, void *context) {
  char buf[10] = {0};
  int ret;
  int n;
  if (SIGRTMIN != sig) return;
  /* 判断串口是否有数据可读 */
  if (POLL_IN == info->si_code) {
    ret = read(fd, buf, 8);  // 一次最多读 8 个字节数据
    printf("[ ");
    for (n = 0; n < ret; n++) printf("%c ", buf[n]);
    printf("]\n");
  }
}
/**
** 异步 I/O 初始化函数
**/
static void async_io_init(void) {
  struct sigaction sigatn;
  int flag;
  /* 使能异步 I/O */
  flag = fcntl(fd, F_GETFL);  // 使能串口的异步 I/O 功能
  flag |= O_ASYNC;
  fcntl(fd, F_SETFL, flag);
  /* 设置异步 I/O 的所有者 */
  fcntl(fd, F_SETOWN, getpid());
  /* 指定实时信号 SIGRTMIN 作为异步 I/O 通知信号 */
  fcntl(fd, F_SETSIG, SIGRTMIN);
  /* 为实时信号 SIGRTMIN 注册信号处理函数 */
  sigatn.sa_sigaction = io_handler;  // 数据可读时，会跳转到 io_handler 函数
  sigatn.sa_flags = SA_SIGINFO;
  sigemptyset(&sigatn.sa_mask);
  sigaction(SIGRTMIN, &sigatn, NULL);
}
int main(int argc, char *argv[]) {
  char *device = "/dev/device2";
  char w_buf[10] = "hello\r\n";  // 通过串口发送出去的数据
  /* 解析出参数 */
  if (NULL == device) {
    fprintf(stderr, "Error: the device and read|write type must be set!\n");
    // show_help(argv[0]);
    exit(EXIT_FAILURE);
  }
  /* 串口初始化 */
  if (uart_init(device)) exit(EXIT_FAILURE);
  /* 串口配置 */
  if (uart_cfg()) {
    tcsetattr(fd, TCSANOW, &sOldConfig);  // 恢复到之前的配置
    close(fd);
    exit(EXIT_FAILURE);
  }
  /* 读|写串口 */

  async_io_init();  // 异步IO
  for (int i = 0; i < 10; ++i) {
    write(fd, w_buf, 10);
    sleep(10);
  }

  /* 退出 */
  tcsetattr(fd, TCSANOW, &sOldConfig);  // 恢复到之前的配置
  close(fd);
  exit(EXIT_SUCCESS);
}
