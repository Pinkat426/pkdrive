//
// Created by HP on 2023/12/30.
//

#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include "common_inc.h"
#include <cstdint>

// #define LCD_CS_H HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin,
// GPIO_PIN_SET)
// #define LCD_CS_L HAL_GPIO_WritePin(LCD_CS_GPIO_Port,
// LCD_CS_Pin, GPIO_PIN_RESET)
// #define LCD_CMD HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin,
// GPIO_PIN_RESET) #define LCD_DATA HAL_GPIO_WritePin(LCD_RS_GPIO_Port,
// LCD_RS_Pin, GPIO_PIN_SET)
#define LCD_CMD LCD_RS_GPIO_Port->BSRR = (uint32_t)LCD_RS_Pin << 16U
#define LCD_DATA LCD_RS_GPIO_Port->BSRR = (uint32_t)LCD_RS_Pin
#define LCD_CS_H LCD_CS_GPIO_Port->BSRR = (uint32_t)LCD_CS_Pin
#define LCD_CS_L LCD_CS_GPIO_Port->BSRR = (uint32_t)LCD_CS_Pin << 16U

class LCD_Driver {

public:
  LCD_Driver();
  void Init(uint8_t USE_HORIZONTAL);

  void _spi_mode_set();

public:
  void fill(uint16_t color);
  void fillpart(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                uint16_t color);
  void draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

  void draw_point(uint16_t x, uint16_t y, uint16_t color);

  void draw_rectangle_Aim(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                          uint16_t *buff);

  void draw_showchar(uint16_t x, uint16_t y, uint8_t chr, uint8_t size,
                     uint16_t color, uint16_t bgcolor);
  void draw_showstring(uint16_t x, uint16_t y, uint8_t *str, uint8_t size,
                       uint16_t width, uint16_t height, uint16_t color,
                       uint16_t bgcolor);

  void draw_shownum(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                    uint8_t size, uint16_t color, uint16_t bgcolor);

private:
  void _spi_send(uint8_t *data, uint16_t len); // spi 发送数据
  void _spi_sendDMA(uint8_t *data, uint16_t len);
  void _lcd_cmd(uint8_t cmd);
  void _lcd_data(uint8_t data);
  void _lcd_data16(uint16_t data);
  void _lcd_datas(uint8_t *data, uint16_t len);
  void _lcd_datasDMA(uint8_t *data, uint16_t len);

  void _lcd_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

public:
  uint8_t flag;
  uint16_t lcd_w;
  uint16_t lcd_h;

  //  uint16_t lcd_buff[172*320];
private:
  uint8_t HORIZONTAL;
};

#endif // LCD_DRIVER_H
