#ifndef COMMON_INC_H
#define COMMON_INC_H

#define CONFIG_FW_VERSION 1.0
#define PRINT(window, fmt, args...) usb_printf(#window ":" fmt "\r\n", ##args)

/*---------------------------- C Scope ---------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "adc.h"
#include "cmsis_os.h"
#include "comm_task.h"
#include "foc_task.h"
#include "main.h"
#include "task.h"
#include "tim.h"
#include "usbd_cdc_if.h"
#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
}

/*---------------------------- C++ Scope ---------------------------*/

#endif
#endif
