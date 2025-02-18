#ifndef MOTOR_H
#define MOTOR_H

#include "as5047p.h"
#include "foc_core.h"
#include <stdint.h>

typedef enum Ctrl_State {
  IDLE = 0,
  Motor_OPEN_MODE,
  Motor_I_MODE,
  Motor_SPEED_MODE,
  Motor_POSITION_MODE,
} Ctrl_State;

typedef struct Motor_State {
  Ctrl_State ctrlState;
} Motor_State;

class Motor {
public:
  Motor();
  void add_AS5047_Driver(AS5047P *as5047_L, AS5047P *as5047_R);
  void start_pwm();
  void enable_current_sampling(uint8_t enable_L, uint8_t enable_R);
  void open_deal();
  void main_deal(uint8_t enable_L, uint8_t enable_R, uint32_t data1,
                 uint32_t data2, uint32_t data3, uint32_t data4);
  void set_position(uint8_t number, float position);

private:
  PKFOC focL; // 左电机FOC控制
  PKFOC focR; // 右电机FOC控制
  Motor_State Lstate, Rstate;

  uint32_t Send_Detail_cnt;
  uint8_t Send_Detail_Flag;
};

#endif /* MOTOR_H */