#ifndef COMMON_INC_H
#define COMMON_INC_H

/*---------------------------- C Scope ---------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "adc.h"
#include "cmsis_os.h"
#include "comm_define.h"
#include "comm_task.h"
#include "foc_task.h"
#include "gpio.h"

#include "lcd_task.h"
#include "main.h"
#include "spi.h"
#include "stdio.h"
#include "task.h"
#include "tim.h"
#include "usbd_cdc_if.h"
#include <foc_task.h>
#include <math.h>
#include <queue.h>
#include <stdint.h>
#include <usbd_cdc_if.h>

#define CONFIG_FW_VERSION 1.0
#define PRINT(window, fmt, args...) usb_printf(#window ":" fmt "\r\n", ##args)

#define up_mechanical_offset 87.5698f
#define down_mechanical_offset 333.7734f

#ifdef __cplusplus
}

/*---------------------------- C++ Scope ---------------------------*/

#endif
#endif
