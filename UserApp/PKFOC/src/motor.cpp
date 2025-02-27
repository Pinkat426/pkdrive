#include "motor.h"
#include "common_inc.h"
#include "spi.h"

extern SPI_HandleTypeDef hspi3;
extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim8;
extern ADC_HandleTypeDef hadc2;

Motor::Motor() : focL(&hspi3, &htim1, &hadc1), focR(&hspi2, &htim8, &hadc2) {
  focL.set_tim(&htim1, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3,
               TIM_CHANNEL_4);
  focL.set_adc(&hadc1);

  focR.set_tim(&htim8, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3,
               TIM_CHANNEL_4);
  focR.set_adc(&hadc2);

  focL.set_mechnical_offset(up_mechanical_offset);
  focR.set_mechnical_offset(down_mechanical_offset);
  // 初始化电机状态

  Lstate.ctrlState = Motor_POSITION_MODE;
  Rstate.ctrlState = Motor_POSITION_MODE;

  Send_Detail_cnt = 0;
  Send_Detail_Flag = DISABLE;
}

void Motor::add_AS5047_Driver(AS5047P *as5047_L, AS5047P *as5047_R) {
  focL.set_endocer(as5047_L);
  focR.set_endocer(as5047_R);
}

void Motor::start_pwm() {
  focL.start_pwm();
  focR.start_pwm();
}

void Motor::enable_current_sampling(uint8_t enable_L, uint8_t enable_R) {
  if (enable_L == ENABLE)
    focL.start_adc_sample();
  if (enable_R == ENABLE)
    focR.start_adc_sample();
}

void Motor::open_deal() {
  // 开环状态下的处理逻辑
}

void Motor::main_deal(uint8_t enable_L, uint8_t enable_R, uint32_t data1,
                      uint32_t data2, uint32_t data3, uint32_t data4) {
  if (enable_L == ENABLE) {
    focL.update_I(2, data1, data2);
    focL.my_aim_control();
  }
  if (enable_R == ENABLE) {
    focR.update_I(2, data3, data4);
    focR.ctrl_position_Loop();
  }
}

void Motor::set_position(uint8_t number, float position) {
  if (number == 0) {
    focL.set_position(position);
  } else {
    focR.set_position(position);
  }
}
void Motor::set_aim(float x, float y) {

};