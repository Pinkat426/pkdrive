#include "as5047p.h"
#include "common_inc.h"
#include <cmath>
#include <cstdint>

/* 寄存器地址定义 */
#define AS5047_REG_ANGLECOM 0xFFFF
#define AS5047_REG_ANGLE 0x3FFF
#define AS5047_REG_ERRFL 0x0003
#define AS5047_READ_FLAG 0x4000 // 修正读标志位为bit15
#define AS5047_CMD_NOP 0xC000

/* 错误标志位掩码 */
#define AS5047_ERROR_MASK 0x0007
#define AS5047_PARITY_ERR_MASK 0x8000

extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi2;

AS5047P::AS5047P(SPI_HandleTypeDef *spi, GPIO_TypeDef *CS_Porti,
                 uint16_t CS_Pini) {
  hspi = spi;
  CS_Port = CS_Porti;
  CS_Pin = CS_Pini;
  cnt = 0;
  speed_update = 0;
  speed_rad = 0;
  last_angle = 0;
  pos_gap = 0;
  speed_sum = 0;
  speed_buf[0] = 0;
}

/********************* SPI控制函数 *********************/
void AS5047P::spiCsLow(void) {
  HAL_GPIO_WritePin(CS_Port, CS_Pin, GPIO_PIN_RESET);
}

void AS5047P::spiCsHigh(void) {
  HAL_GPIO_WritePin(CS_Port, CS_Pin, GPIO_PIN_SET);
}

uint16_t AS5047P::spiTransfer(uint16_t txData) {
  uint16_t rxData = 0;

  spiCsLow();
  HAL_SPI_TransmitReceive(hspi, (uint8_t *)&txData, (uint8_t *)&rxData, 1,
                          HAL_MAX_DELAY);
  spiCsHigh();

  return rxData;
}

float x_buf[10] = {0};
float y_buf[10] = {0};
float x_sum = 0.0f;
float y_sum = 0.0f;
/********************* 核心功能函数 *********************/
float AS5047P::readAngle(bool enableDaec) {
  float angle = 0.0f;
  const uint16_t cmd =
      enableDaec ? AS5047_REG_ANGLECOM : (AS5047_REG_ANGLE | AS5047_READ_FLAG);

  uint16_t rawData;

  rawData = spiTransfer(cmd);
  rawData = spiTransfer(AS5047_CMD_NOP); // 读取数据需要NOP命令

  /* 校验数据有效性 */
  if ((rawData & AS5047_PARITY_ERR_MASK) && !checkParity(rawData)) {
    handleError(rawData & AS5047_ERROR_MASK);
  }
  angle = ((((float)(rawData & 0x3FFF) * 360.0f) / 16384.0f) - 180.0f) * 2.0f;
  float radians = angle * M_PI / 180.0f;

  float x_new = cos(radians);
  float y_new = sin(radians);

  // 更新滑窗和总和
  x_sum -= x_buf[0];
  y_sum -= y_buf[0];

  // 滑动数组元素
  for (uint8_t i = 0; i < 9; i++) {
    x_buf[i] = x_buf[i + 1];
    y_buf[i] = y_buf[i + 1];
  }

  // 存入新分量并更新总和
  x_buf[9] = x_new;
  y_buf[9] = y_new;
  x_sum += x_new;
  y_sum += y_new;

  // 计算平均向量对应的角度
  float avg_x = x_sum / 10.0f;
  float avg_y = y_sum / 10.0f;

  float filtered_angle = atan2f(avg_y, avg_x) * 180.0f / M_PI;

  if (filtered_angle < 0) {
    filtered_angle += 360.0f;
  }

  angle = filtered_angle;
  // PRINT(foc, "%.3f", angle);
  cnt += 1;
  if (cnt == 1) {
    float angle_error = (angle - last_angle);
    if (angle_error > 180) {
      angle_error -= 360;
    } else if (angle_error < -180) {
      angle_error += 360;
    }
    speed_rad =
        ((angle_error) * 1000) * 0.017453292f; // 弧度  speed_rad * 0.2f + 0.8f
                                               // *    0.166751197f * 1000;//RPM
    pos_gap = angle_error;
    speed_sum -= speed_buf[0];
    for (uint8_t i = 0; i < 9; i++) {
      speed_buf[i] = speed_buf[i + 1];
    }
    speed_buf[9] = speed_rad;
    speed_sum += speed_buf[9];
    speed_rad = speed_sum / 10.0f;
    last_angle = angle;
    // PRINT(foc, "%.3f,%.3f", angle_error, speed_rad);
    cnt = 0;
    speed_update = 1;
  }
  return angle;
}

float AS5047P::readSpeed() {
  speed_update = 0;
  return speed_rad;
}

/********************* 辅助函数 *********************/
bool AS5047P::checkParity(uint16_t data) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < 15; i++) {
    if (data & (1 << i))
      count++;
  }
  return (count % 2) == ((data >> 15) & 0x01);
}

void AS5047P::handleError(uint8_t errorCode) { lastError = errorCode; }

/********************* 调试接口 *********************/
void AS5047P::debugOutput() {
  uint16_t rawAngle = spiTransfer(AS5047_REG_ANGLE | AS5047_READ_FLAG);
  rawAngle = spiTransfer(AS5047_CMD_NOP);
  usb_printf("Raw Angle: 0x%04X\n", rawAngle);
}
