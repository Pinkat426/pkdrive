//
// Created by Valentina_HP on 2023/3/17.
//

#include "IIC.h"

//__attribute__((optimize("O0")))
void IIC::Init(GPIO_TypeDef *GPIO_CLK, GPIO_TypeDef *GPIO_SDA,
               uint16_t GPIO_Pin_CLK, uint16_t GPIO_Pin_SDA) {
  _GPIO_CLK = GPIO_CLK;
  _GPIO_SDA = GPIO_SDA;
  _GPIO_Pin_CLK = GPIO_Pin_CLK;
  _GPIO_Pin_SDA = GPIO_Pin_SDA;

  _SDA_Out();
  _SDA_High();
  _SCL_High();
}
void IIC::_iic_delay() {
  volatile int time = 10;
  while (time--)
    ;
}

void IIC::_SDA_Out() {
  HAL_GPIO_DeInit(_GPIO_SDA, _GPIO_Pin_SDA);

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = _GPIO_Pin_SDA;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(_GPIO_SDA, &GPIO_InitStruct);
}
void IIC::_SDA_In() {
  HAL_GPIO_DeInit(_GPIO_SDA, _GPIO_Pin_SDA);
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = _GPIO_Pin_SDA;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(_GPIO_SDA, &GPIO_InitStruct);
}

void IIC::_SDA_High() {
  HAL_GPIO_WritePin(_GPIO_SDA, _GPIO_Pin_SDA, GPIO_PIN_SET);
}
void IIC::_SDA_Low() {
  HAL_GPIO_WritePin(_GPIO_SDA, _GPIO_Pin_SDA, GPIO_PIN_RESET);
}
void IIC::_SCL_High() {
  HAL_GPIO_WritePin(_GPIO_CLK, _GPIO_Pin_CLK, GPIO_PIN_SET);
}
void IIC::_SCL_Low() {
  HAL_GPIO_WritePin(_GPIO_CLK, _GPIO_Pin_CLK, GPIO_PIN_RESET);
}

uint8_t IIC::_SDA_Read() { return HAL_GPIO_ReadPin(_GPIO_SDA, _GPIO_Pin_SDA); }
void IIC::_IIC_Start() {
  _SDA_Out();
  _SDA_High();
  _SCL_High();
  _iic_delay();
  _SDA_Low();
  _iic_delay();
}
void IIC::_IIC_Stop() {
  _SDA_Out();
  _SCL_Low();
  _iic_delay();
  _SDA_Low();
  _iic_delay();
  _SCL_High();
  _iic_delay();
  _SDA_High();
  _iic_delay();
}

void IIC::_IIC_Write(uint8_t data) {
  uint8_t i;

  _SDA_Out();
  _iic_delay();
  for (i = 0; i < 8; i++) {
    _SCL_Low();
    _iic_delay();
    if (data & 0x80)
      _SDA_High();
    else
      _SDA_Low();
    data = data << 1;
    _iic_delay();
    _SCL_High();
    _iic_delay();
  }
  _SCL_Low();
}
uint8_t IIC::_IIC_Read() {
  uint8_t i = 0;
  uint8_t data = 0x00;
  _SDA_In();
  _iic_delay();

  for (i = 0; i < 8; i++) {
    data <<= 1;
    _SCL_High();
    _iic_delay();
    data |= _SDA_Read();
    _iic_delay();
    _SCL_Low();
    _iic_delay();
  }

  return data;
}

bool IIC::_IIC_Ack() {
  unsigned int time = 0xf;

  _SDA_Out();
  _SCL_Low();
  _iic_delay();
  _SDA_High();

  _SDA_In();
  _iic_delay();
  _SCL_High();
  _iic_delay();
  while (time) {
    if (_SDA_Read() == 0) // 读取成功
    {
      _iic_delay();
      _SCL_Low();
      //    usb_printf("iic ack ok\r\n");
      return true;
    }
    time--;
  }
  //  usb_printf("iic ack fail\r\n");
  _SCL_Low();
  _iic_delay();
  return false; // 读取错误
}

void IIC::iic_write_data(unsigned char device, unsigned char registers,
                         unsigned char value) {
  _IIC_Start();
  _IIC_Write(device);
  _IIC_Ack();
  //   ESP_LOGI("iic test1","%d %d\r\n", mpu6050_iic_read_ack(),
  //   gpio_get_level(0));
  _IIC_Write(registers);
  _IIC_Ack();
  //  ESP_LOGI("iic test1","%d %d\r\n", mpu6050_iic_read_ack(),
  //  gpio_get_level(0));
  _IIC_Write(value);
  _IIC_Ack();
  // ESP_LOGI("iic test1","%d %d\r\n", mpu6050_iic_read_ack(),
  // gpio_get_level(0));
  _IIC_Stop();
}
uint8_t IIC::iic_read_data(unsigned char device, unsigned char registers) {
  uint8_t data;
  _IIC_Start();
  _IIC_Write(device);
  _IIC_Ack();
  _IIC_Write(registers);
  _IIC_Ack();
  _IIC_Start();
  _IIC_Write(device + 0x01);
  _IIC_Ack();
  data = _IIC_Read();
  _IIC_Ack();
  _IIC_Stop();
  return data;
}