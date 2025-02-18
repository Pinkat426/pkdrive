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
float speed_buf2[10] = {0};
float speed_sum2 = 0.0f;
/********************* 核心功能函数 *********************/
float AS5047P::readAngle(bool enableDaec) {
  float angle = 0.0f;
  const uint16_t cmd =
      enableDaec ? AS5047_REG_ANGLECOM : (AS5047_REG_ANGLE | AS5047_READ_FLAG);

  uint16_t rawData;

  rawData = spiTransfer(cmd);
  rawData = spiTransfer(AS5047_CMD_NOP); // 读取数据需要NOP命令
  // rawData = 0x3FFF;
  /* 校验数据有效性 */
  if ((rawData & AS5047_PARITY_ERR_MASK) && !checkParity(rawData)) {
    handleError(rawData & AS5047_ERROR_MASK);
  }
  angle = ((((float)(rawData & 0x3FFF) * 360.0f) / 16384.0f) - 180.0f) * 2.0f;

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
    speed_sum2 -= speed_buf2[0];
    for (uint8_t i = 0; i < 9; i++) {
      speed_buf2[i] = speed_buf2[i + 1];
    }
    speed_buf2[9] = speed_rad;
    speed_sum2 += speed_buf2[9];
    speed_rad = speed_sum2 / 10.0f;

    last_angle = angle;
    // PRINT(foc, "%.3f,%.3f", angle_error, speed_rad);
    cnt = 0;
    speed_update = 1;
  }

  // PRINT(foc, "%.3f", angle);

  return angle;
}

float AS5047P::readSpeed() {
  speed_update = 0;
  return speed_rad;
}

float AS5047P::readPositionGap() { return pos_gap; }

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
