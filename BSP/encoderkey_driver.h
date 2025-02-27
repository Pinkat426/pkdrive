//
// Created by HP on 2024/1/1.
//

#ifndef AURORA_ST_ENCODERKEY_DRIVER_H
#define AURORA_ST_ENCODERKEY_DRIVER_H

#include "common_inc.h"

// EncoderKey 状态
typedef enum EncoderKeyState {
  EncoderKey_None = 0,
  EncoderKey_Click,  // 单击
  EncoderKey_LClick, // 长按
  EncoderKey_DClick, // 双击
  EncoderKey_Left,   // 左边
  EncoderKey_Right   // 右边
} EncoderKeyState;

class EncoderKey_Driver {

public:
  EncoderKeyState get_key_state(); // 获取按键状态

  void EncoderKey_EXTI_Callback(uint16_t GPIO_Pin);
  void detect_key_state();

private:
  EncoderKeyState key_state;
  uint32_t key_tick;
  uint8_t dclick_flag;
  uint8_t ec_flag;
};

#endif // AURORA_ST_ENCODERKEY_DRIVER_H
