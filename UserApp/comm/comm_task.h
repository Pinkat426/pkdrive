#ifndef COMM_TASK_H
#define COMM_TASK_H

#ifdef __cplusplus
extern "C" {
#endif
#include "main.h"
void comm_task(void);
void msg_deal(uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}

#endif
#endif /* FOC_TASK_H */
