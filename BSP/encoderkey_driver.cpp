//
// Created by HP on 2024/1/1.
//

#include "encoderkey_driver.h"
#include "stm32f4xx_hal_gpio.h"





/* 获取按键状态 如果按键按下 则会清除按键状态 */
EncoderKeyState EncoderKey_Driver::get_key_state() {
  EncoderKeyState state;
  if (key_state == EncoderKey_Click) {
    if (((int32_t)HAL_GetTick() - (int32_t)key_tick < 450 ||
         (int32_t)key_tick - (int32_t)HAL_GetTick() > 450))
      return EncoderKey_None;
  }
  state = key_state;
  key_state = EncoderKey_None;
  return state;
}

void EncoderKey_Driver::EncoderKey_Detect() {

  if (HAL_GPIO_ReadPin(EC_KEY_GPIO_Port, EC_KEY_Pin) == 0) {
    if (HAL_GetTick() - key_tick < 450)
      dclick_flag = 1;
  } else {
    if (HAL_GetTick() - key_tick > 1500) {
    } // 长按
  }
}
