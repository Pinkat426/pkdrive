#include "foc_task.h"
#include "as5047p.h"
#include "comm_define.h"
#include "common_inc.h"
#include "motor.h"
#include "queue.h"
#include "stm32f4xx.h"
#include <cstdint>
#include <stdint.h>

extern SPI_HandleTypeDef hspi3; // 添加这行
extern SPI_HandleTypeDef hspi2; // 添加这行
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2; // 添加这行
extern QueueHandle_t xFocQueue;

#define PWM_CLOCK_HZ 42000000                          // 定时器时钟频率
#define PWM_PERIOD 4199                                // ARR值
#define PWM_FREQ (PWM_CLOCK_HZ / (PWM_PERIOD + 1) / 2) // ≈5kHz
#define PWM_AMPLITUDE 1800 // 幅值（最大不超过2099）
#define ELECTRICAL_2PI (2 * M_PI)
#define POLE_PAIRS 11 // 修改为实际极对数
#define mechanical_pi(x) ((x) * M_PI)
#define target_rpm 1.0f      // 示例转速
#define control_period 0.01f // 控制周期1ms
#define delta_theta (target_rpm * 2 * M_PI / 60) * POLE_PAIRS *control_period

uint32_t dwt_start, dwt_end;
volatile float theta = 0.0f;
uint32_t start, end;
uint16_t dir_start, dir_end;
uint32_t I_value[2];
volatile float voltage0, voltage1;
#define INA240_GAIN 50.0f      // INA240A2 增益
#define SHUNT_RESISTOR 0.02f   // 20mΩ 采样电阻
volatile float I_mA_0, I_mA_1; // 最终电流值（毫安）
uint32_t IL_value[2];
uint32_t IR_value[2];
/** adc 注入转换完成中断 **/

uint32_t IL_value0_buf[10];
uint32_t IL_value1_buf[10];
uint32_t IL_value0_sum;
uint32_t IL_value1_sum;
uint32_t run_count = 0;
extern osMessageQueueId_t Queue_ADCHandle;

volatile uint32_t flag_l = 0;
volatile uint32_t flag_r = 0;

// 创建编码器实例
AS5047P encoderL(&hspi3, SPI3_CS_GPIO_Port, SPI3_CS_Pin);
AS5047P encoderR(&hspi2, SPI2_CS_GPIO_Port, SPI2_CS_Pin);

static Motor motor;

volatile uint16_t angle_int;

float angle_float;
float pc_aim_pos = 0.0f;
void foc_task(void) {
  motor.add_AS5047_Driver(&encoderL, &encoderR);
  motor.enable_current_sampling(ENABLE, ENABLE);
  motor.start_pwm();
  FocMsg_t received_msg;
  // FOC线程主循环
  while (1) {
    if (1) {
      if (xQueueReceive(xFocQueue, &received_msg, portMAX_DELAY) == pdPASS) {
        switch (received_msg.type) {
        case AIM_CMD:
          PRINT(WIN, "%d,%d", received_msg.aim.x, received_msg.aim.y);
          // 处理坐标指令
          break;
        case SPD_CMD:
          // 处理速度指令
          PRINT(WIN, "%d", received_msg.speed);
          break;
        case POS_CMD:
          // 处理位置指令
          pc_aim_pos = ((float)received_msg.position) / 1000.0f;
          motor.set_position(0, pc_aim_pos);
          break;
        }
      }
    }
    osDelay(1);
  }
}
uint32_t dwt_cycle;
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc) {

  // 临界区保护
  UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
  run_count++;
  dwt_start = DWT->CYCCNT;
  // 读取ADC值
  if (hadc == &hadc1) {

    IL_value[0] = hadc1.Instance->JDR1; // 获取转换结果
    IL_value[1] = hadc1.Instance->JDR2; // 获取转换结果

    IL_value0_sum -= IL_value0_buf[0];
    IL_value1_sum -= IL_value1_buf[0];
    for (int i = 0; i < 9; i++) {
      IL_value0_buf[i] = IL_value0_buf[i + 1];
      IL_value1_buf[i] = IL_value1_buf[i + 1];
    }
    IL_value0_buf[9] = IL_value[0];
    IL_value1_buf[9] = IL_value[1];
    IL_value0_sum += IL_value[0];
    IL_value1_sum += IL_value[1];
    IL_value[0] = IL_value0_sum / 10;
    IL_value[1] = IL_value1_sum / 10;
    motor.main_deal(ENABLE, DISABLE, IL_value[0], IL_value[1], IR_value[0],
                    IR_value[1]);

    dwt_end = DWT->CYCCNT;
    dwt_cycle = dwt_end - dwt_start;
    if (run_count == 10) {
      run_count = 0;
    }
  } else if (hadc == &hadc2) {
    IR_value[0] = hadc2.Instance->JDR1; // 获取转换结果
    IR_value[1] = hadc2.Instance->JDR2; // 获取转换结果
  }
  taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
}
