//
// Created by HP on 2024/1/1.
//

#include "encoderkey_driver.h"

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

void EncoderKey_Driver::EncoderKey_EXTI_Callback(uint16_t GPIO_Pin) {
  // 按键
  if (GPIO_Pin == EC_KEY_Pin) {
    if (HAL_GPIO_ReadPin(EC_KEY_GPIO_Port, EC_KEY_Pin) == 0) {
      if (HAL_GetTick() - key_tick < 450)
        dclick_flag = 1;
      else
        dclick_flag = 0;
      key_tick = HAL_GetTick();
    } else {
      if (HAL_GetTick() - key_tick > 1500) // 长按
        key_state = EncoderKey_LClick;
      else if (HAL_GetTick() - key_tick > 15) {
        key_state = (dclick_flag == 0) ? EncoderKey_Click : EncoderKey_DClick;
      }
      dclick_flag = 0;
    }
  } else if (GPIO_Pin == EC_AR_Pin) { // 旋转检测
    if (ec_flag == 0) {
      if (HAL_GPIO_ReadPin(EC_AR_GPIO_Port, EC_AR_Pin) ==
          1) // 消除误差,防止误判
        ec_flag = 0;
      else if (HAL_GPIO_ReadPin(EC_BR_GPIO_Port, EC_BR_Pin) == 1)
        ec_flag = 1;
      else
        ec_flag = 2;
    } else {
      if (HAL_GPIO_ReadPin(EC_AR_GPIO_Port, EC_AR_Pin) ==
          0) // 消除误差,防止误判
        ;    // ec_flag = 0;
      else if (ec_flag == 1 &&
               HAL_GPIO_ReadPin(EC_BR_GPIO_Port, EC_BR_Pin) == 0)
        key_state = EncoderKey_Right;
      else if (ec_flag == 2 &&
               HAL_GPIO_ReadPin(EC_BR_GPIO_Port, EC_BR_Pin) == 1)
        key_state = EncoderKey_Left;
      ec_flag = 0;
    }
  }
}

void EncoderKey_Driver::detect_key_state() {
  // 按键检测
  if (HAL_GPIO_ReadPin(EC_KEY_GPIO_Port, EC_KEY_Pin) == 0) { // 按下
    if (HAL_GetTick() - key_tick < 450)
      dclick_flag = 1;
    else
      dclick_flag = 0;
    key_tick = HAL_GetTick();
  } else {                               // 松开
    if (HAL_GetTick() - key_tick > 1500) // 长按
      key_state = EncoderKey_LClick;
    else if (HAL_GetTick() - key_tick > 15) {
      key_state = (dclick_flag == 0) ? EncoderKey_Click : EncoderKey_DClick;
    }
    dclick_flag = 0;
  }

  // 旋转检测
  if (ec_flag == 0) {
    if (HAL_GPIO_ReadPin(EC_AR_GPIO_Port, EC_AR_Pin) == 1)
      ec_flag = 0;
    else if (HAL_GPIO_ReadPin(EC_BR_GPIO_Port, EC_BR_Pin) == 1)
      ec_flag = 1;
    else
      ec_flag = 2;
  } else {
    if (HAL_GPIO_ReadPin(EC_AR_GPIO_Port, EC_AR_Pin) == 0)
      ; // ec_flag = 0;
    else if (ec_flag == 1 && HAL_GPIO_ReadPin(EC_BR_GPIO_Port, EC_BR_Pin) == 0)
      key_state = EncoderKey_Right;
    else if (ec_flag == 2 && HAL_GPIO_ReadPin(EC_BR_GPIO_Port, EC_BR_Pin) == 1)
      key_state = EncoderKey_Left;
    ec_flag = 0;
  }
}