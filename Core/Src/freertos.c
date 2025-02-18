/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "SEGGER_RTT.h"
#include "comm_define.h"
#include "comm_task.h"
#include <foc_task.h>
#include <queue.h>
#include <stdint.h>
#include <usbd_cdc_if.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Task_comm */
osThreadId_t Task_commHandle;
uint32_t defaultTaskBuffer[ 512 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t Task_comm_attributes = {
  .name = "Task_comm",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_foc */
osThreadId_t Task_focHandle;
uint32_t Task_focBuffer[ 512 ];
osStaticThreadDef_t Task_focControlBlock;
const osThreadAttr_t Task_foc_attributes = {
  .name = "Task_foc",
  .cb_mem = &Task_focControlBlock,
  .cb_size = sizeof(Task_focControlBlock),
  .stack_mem = &Task_focBuffer[0],
  .stack_size = sizeof(Task_focBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_led */
osThreadId_t Task_ledHandle;
uint32_t Task_ledBuffer[ 512 ];
osStaticThreadDef_t Task_ledControlBlock;
const osThreadAttr_t Task_led_attributes = {
  .name = "Task_led",
  .cb_mem = &Task_ledControlBlock,
  .cb_size = sizeof(Task_ledControlBlock),
  .stack_mem = &Task_ledBuffer[0],
  .stack_size = sizeof(Task_ledBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Queue_ADC */
osMessageQueueId_t Queue_ADCHandle;
const osMessageQueueAttr_t Queue_ADC_attributes = {
  .name = "Queue_ADC"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
QueueHandle_t xFocQueue;

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Queue_ADC */
  Queue_ADCHandle = osMessageQueueNew (1, sizeof(uint32_t), &Queue_ADC_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  xFocQueue = xQueueCreate(10, sizeof(FocMsg_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_comm */
  Task_commHandle = osThreadNew(StartDefaultTask, NULL, &Task_comm_attributes);

  /* creation of Task_foc */
  Task_focHandle = osThreadNew(StartTask02, NULL, &Task_foc_attributes);

  /* creation of Task_led */
  Task_ledHandle = osThreadNew(StartTask03, NULL, &Task_led_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */

  /* Infinite loop */
  for (;;) {
    // 通信线程
    comm_task();
    // osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
 * @brief Function implementing the Task_foc thread.
 * @param argument: Not used
 * @retval None
 */

/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */

  /* Infinite loop */
  for (;;) {
    // FOC线程
    // SEGGER_RTT_WriteString(0, "task02\n");
    foc_task();
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
 * @brief Function implementing the Task_led thread.
 * @param argument: Not used
 * @retval None
 */

/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  uint32_t kkk = 0;

  /* Infinite loop */
  for (;;) {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);
    osDelay(1000);
  }
  /* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

