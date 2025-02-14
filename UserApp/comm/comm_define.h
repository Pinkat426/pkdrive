#ifndef COMM_DEFINE_H
#define COMM_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "common_inc.h"

#define CMD_HEAD 0x68 // 'h'
#define CMD_TAIL 0x74 // 't'

typedef enum {
  AIM_CMD,
  SPD_CMD,
  POS_CMD,
} FocMsgType_t;

// 定义消息结构体
typedef struct {
  FocMsgType_t type;
  union {
    struct {
      int32_t x, y;
    } aim;
    int32_t speed;
    int32_t position;
  };
} FocMsg_t;

#ifdef __cplusplus
}
#endif

#endif /* COMM_DEFINE_H */