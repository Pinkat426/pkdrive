#ifndef AS5047P_H
#define AS5047P_H

/* 硬件依赖 */
#include "stm32f4xx_hal.h"
#include <cmath>
#include <math.h>
#include <stdbool.h>

class AS5047P {
public:
  /* 错误代码定义 */
  enum ErrorCode {
    NO_ERROR = 0x00,
    FRAMING_ERROR = 0x01,
    INVALID_COMMAND = 0x02,
    PARITY_ERROR = 0x04
  };

  /* 设备操作模式 */
  enum OpMode {
    NORMAL_MODE = 0,
    DAEC_MODE = 1 // 动态角度误差补偿模式
  };

  AS5047P(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_Port, uint16_t CS_Pin);

  /* 核心功能接口 */
  float readAngle(bool enableDaec = false);
  float readSpeed();
  void debugOutput();
  float readPositionGap();
  uint8_t as5047_read_speed_update() { return speed_update; };
  /* 诊断接口 */
  uint8_t getLastError() const { return lastError; }
  void clearErrors() { lastError = NO_ERROR; }

private:
  /* 寄存器地址定义 */
  static constexpr uint16_t REG_ANGLECOM = 0xFFFF;
  static constexpr uint16_t REG_ANGLE = 0x3FFF;
  static constexpr uint16_t REG_ERRFL = 0x0003;
  static constexpr uint16_t READ_FLAG = 0x4000;
  static constexpr uint16_t CMD_NOP = 0xC000;

  /* 错误标志位掩码 */
  static constexpr uint16_t ERROR_MASK = 0x0007;
  static constexpr uint16_t PARITY_ERR_MASK = 0x8000;

  SPI_HandleTypeDef *hspi;
  uint8_t lastError;
  GPIO_TypeDef *CS_Port;
  uint16_t CS_Pin;
  uint8_t cnt;
  uint8_t speed_update;
  float speed_rad;
  float last_angle;
  float pos_gap;

public:
  float speed_buf[10];
  float speed_sum;
  float angle_buf[10];
  float angle_sum;

private:
  /* 私有辅助函数 */
  /* 私有辅助函数 */
  void spiCsLow();
  void spiCsHigh();
  uint16_t spiTransfer(uint16_t txData);
  bool checkParity(uint16_t data);
  void handleError(uint8_t errorCode);
  uint16_t rawRead(uint16_t registerAddress);
};

#endif /* AS5047P_H */
