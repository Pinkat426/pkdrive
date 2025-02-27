#include "comm_task.h"
#include "comm_define.h"
#include "common_inc.h"
#include "freertos.h"
#include "queue.h"

extern QueueHandle_t xFocQueue;
void comm_task(void) {
  // 创建队列（建议长度5-10）

  while (1) {
    // 临界区保护
    taskENTER_CRITICAL();

    if (usbcdc_recvlen > 0) {
      msg_deal(usbcdc_recvbuff, usbcdc_recvlen);
      usbcdc_recvlen = 0;
    }

    taskEXIT_CRITICAL();
    osDelay(1);
  }
}

void msg_deal(uint8_t *buf, uint32_t len) {
  FocMsg_t new_msg;
  BaseType_t xStatus;

  if (len != 24) {
    usb_printf("CMD LEN ERROR \r\n");
    return;
  } else if (buf[0] != CMD_HEAD || buf[23] != CMD_TAIL) {
    usb_printf("CMD HEAD OR TAIL ERROR \r\n");
    return;
  }

  if (buf[1] == 'a' && buf[2] == 'i' && buf[3] == 'm') {
    // usb_printf("CMD AIM \r\n");
    /* 坐标数据解析 */
    char x_str[7] = {0}; // 6字符+结束符
    char y_str[7] = {0};
    int32_t x = 0, y = 0;

    // 提取X坐标（4-9字节）和Y坐标（10-15字节）
    memcpy(x_str, &buf[4], 6);
    memcpy(y_str, &buf[10], 6);

    // 转换为整型（带符号）
    x = atoi(x_str);
    y = atoi(y_str);
    new_msg.type = AIM_CMD;
    new_msg.aim.x = x;
    new_msg.aim.y = y;

    xStatus = xQueueSendToBack(xFocQueue, &new_msg, 0);
  }

  if (buf[1] == 's' && buf[2] == 'p' && buf[3] == 'd') {

    /* 速度数据解析 */
    char spd_str[7] = {0}; // 6字符+结束符

    int32_t spd = 0;

    // 提取X坐标（4-9字节）和Y坐标（10-15字节）
    memcpy(spd_str, &buf[4], 6);

    // 转换为整型（带符号）
    spd = atoi(spd_str);
    // PRINT(spd, "%d", spd);

    new_msg.type = SPD_CMD;
    new_msg.position = spd;

    xStatus = xQueueSendToBack(xFocQueue, &new_msg, 0);
    if (xStatus != pdPASS) {
      usb_printf("Queue Full!\r\n");
    }
    // 向队列发送消息
  }

  if (buf[1] == 'p' && buf[2] == 'o' && buf[3] == 's') {
    // usb_printf("CMD POS \r\n");
    /* 位置数据解析 */
    char pos_str[7] = {0}; // 6字符+结束符

    int32_t pos = 0;

    // 提取X坐标（4-9字节）和Y坐标（10-15字节）
    memcpy(pos_str, &buf[4], 6);

    // 转换为整型（带符号）
    pos = atoi(pos_str);

    new_msg.type = POS_CMD;
    new_msg.position = pos;

    xStatus = xQueueSendToBack(xFocQueue, &new_msg, 0);
  }
  if (buf[1] == 'p' && buf[2] == 'o' && buf[3] == 's' && buf[4] == 'p') {
    // usb_printf("CMD MOD \r\n");
    /* 模式数据解析 */
    char mod_str[13] = {0}; // 6字符+结束符
    int32_t mod = 0;

    // 提取X坐标（4-9字节）和Y坐标（10-15字节）
    memcpy(mod_str, &buf[5], 12);

    // 转换为整型（带符号）
    mod = atoi(mod_str);

    // PRINT(mod, "%d", mod);
  }
}
