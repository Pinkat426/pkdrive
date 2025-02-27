//
// Created by HP on 2023/12/30.
//

#include "lcd_driver.h"
#include "font.h"
#include <cstdint>

uint16_t lcd_buff[320 * 80];

LCD_Driver::LCD_Driver() {}

void LCD_Driver::Init(uint8_t USE_HORIZONTAL) {
  LCD_CS_H;
  LCD_CMD;
  HORIZONTAL = USE_HORIZONTAL;
  if (HORIZONTAL == 0 || HORIZONTAL == 1) {
    lcd_w = 172;
    lcd_h = 320;
  } else {
    lcd_w = 320;
    lcd_h = 172;
  }

  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(30);
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  //  HAL_GPIO_WritePin(LCD_BLK_GPIO_Port,LCD_BLK_Pin,GPIO_PIN_RESET);

  _lcd_cmd(0x11);
  _lcd_cmd(0x36);
  if (USE_HORIZONTAL == 0)
    _lcd_data(0x00);
  else if (USE_HORIZONTAL == 1)
    _lcd_data(0xC0);
  else if (USE_HORIZONTAL == 2)
    _lcd_data(0x70);
  else
    _lcd_data(0xA0);

  _lcd_cmd(0x3A);
  _lcd_data(0x05);

  _lcd_cmd(0xB2);
  _lcd_data(0x0C);
  _lcd_data(0x0C);
  _lcd_data(0x00);
  _lcd_data(0x33);
  _lcd_data(0x33);

  _lcd_cmd(0xB7);
  _lcd_data(0x35);

  _lcd_cmd(0xBB);
  _lcd_data(0x35);

  _lcd_cmd(0xC0);
  _lcd_data(0x2C);

  _lcd_cmd(0xC2);
  _lcd_data(0x01);

  _lcd_cmd(0xC3);
  _lcd_data(0x13);

  _lcd_cmd(0xC4);
  _lcd_data(0x20);

  _lcd_cmd(0xC6);
  _lcd_data(0x0F);

  _lcd_cmd(0xD0);
  _lcd_data(0xA4);
  _lcd_data(0xA1);

  _lcd_cmd(0xD6);
  _lcd_data(0xA1);

  _lcd_cmd(0xE0);
  _lcd_data(0xF0);
  _lcd_data(0x00);
  _lcd_data(0x04);
  _lcd_data(0x04);
  _lcd_data(0x04);
  _lcd_data(0x05);
  _lcd_data(0x29);
  _lcd_data(0x33);
  _lcd_data(0x3E);
  _lcd_data(0x38);
  _lcd_data(0x12);
  _lcd_data(0x12);
  _lcd_data(0x28);
  _lcd_data(0x30);

  _lcd_cmd(0xE1);
  _lcd_data(0xF0);
  _lcd_data(0x07);
  _lcd_data(0x0A);
  _lcd_data(0x0D);
  _lcd_data(0x0B);
  _lcd_data(0x07);
  _lcd_data(0x28);
  _lcd_data(0x33);
  _lcd_data(0x3E);
  _lcd_data(0x36);
  _lcd_data(0x14);
  _lcd_data(0x14);
  _lcd_data(0x29);
  _lcd_data(0x32);

  // 	_lcd_cmd(0x2A);
  //	__lcd_data(0x00);
  //	__lcd_data(0x22);
  //	__lcd_data(0x00);
  //	__lcd_data(0xCD);
  //	__lcd_data(0x2B);
  //	__lcd_data(0x00);
  //	__lcd_data(0x00);
  //	__lcd_data(0x01);
  //	__lcd_data(0x3F);
  //	_lcd_cmd(0x2C);
  _lcd_cmd(0x21);

  _lcd_cmd(0x11);
  HAL_Delay(120);
  _lcd_cmd(0x29);
}

void LCD_Driver::_spi_mode_set() {

  if (HAL_SPI_DeInit(&hspi1) != HAL_OK) {
    while (true)
      usb_printf("lcd spi deinit error\r\n");
  }
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) {
    while (true)
      usb_printf("lcd spi deinit error\r\n");
  }
}

void LCD_Driver::_spi_send(uint8_t *data, uint16_t len) {
  //_spi_mode_set();
  LCD_CS_L;
  HAL_SPI_Transmit(&hspi1, data, len, 0xffff);
  LCD_CS_H;
}
void LCD_Driver::_spi_sendDMA(uint8_t *data, uint16_t len) {
  //_spi_mode_set();
  LCD_CS_L;
  HAL_SPI_Transmit_DMA(&hspi1, data, len);
  while (HAL_DMA_GetState(hspi1.hdmatx) != HAL_DMA_STATE_READY)
    ;
  LCD_CS_H;
}
void LCD_Driver::_lcd_cmd(uint8_t cmd) {
  LCD_CMD;
  //_spi_send(&cmd, 1);
  _spi_sendDMA(&cmd, 1);
}
void LCD_Driver::_lcd_data(uint8_t data) {
  LCD_DATA;
  _spi_sendDMA(&data, 1);
}
void LCD_Driver::_lcd_data16(uint16_t data) {
  _lcd_data(data >> 8);
  _lcd_data(data);
}
void LCD_Driver::_lcd_datas(uint8_t *data, uint16_t len) {
  LCD_DATA;
  _spi_send(data, len);
}
void LCD_Driver::_lcd_datasDMA(uint8_t *data, uint16_t len) {
  LCD_DATA;
  _spi_sendDMA(data, len);
}

void LCD_Driver::_lcd_set_address(uint16_t x1, uint16_t y1, uint16_t x2,
                                  uint16_t y2) {
  if (HORIZONTAL == 0) {
    _lcd_cmd(0x2a);
    _lcd_data16(x1 + 34);
    _lcd_data16(x2 + 34);
    _lcd_cmd(0x2b);
    _lcd_data16(y1);
    _lcd_data16(y2);
    _lcd_cmd(0x2c);
  } else if (HORIZONTAL == 1) {
    _lcd_cmd(0x2a);
    _lcd_data16(x1 + 34);
    _lcd_data16(x2 + 34);
    _lcd_cmd(0x2b);
    _lcd_data16(y1);
    _lcd_data16(y2);
    _lcd_cmd(0x2c);
  } else if (HORIZONTAL == 2) {
    _lcd_cmd(0x2a);
    _lcd_data16(x1);
    _lcd_data16(x2);
    _lcd_cmd(0x2b);
    _lcd_data16(y1 + 34);
    _lcd_data16(y2 + 34);
    _lcd_cmd(0x2c);
  } else {
    _lcd_cmd(0x2a);
    _lcd_data16(x1);
    _lcd_data16(x2);
    _lcd_cmd(0x2b);
    _lcd_data16(y1 + 34);
    _lcd_data16(y2 + 34);
    _lcd_cmd(0x2c);
  }
}

void LCD_Driver::fill(uint16_t color) {
  uint32_t i;

  for (i = 0; i < 172 * 320 / 32; i++)
    lcd_buff[i] = color;
  _spi_mode_set();
  _lcd_set_address(0, 0, lcd_w - 1, lcd_h - 1);
  LCD_DATA;
  LCD_CS_L;
  for (i = 0; i < 32; i++) {
    HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)lcd_buff, 320 * 172 / 16);
    while (HAL_DMA_GetState(hspi1.hdmatx) != HAL_DMA_STATE_READY)
      ;
  }
  LCD_CS_H;
}
// 局部填充
void LCD_Driver::fillpart(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                          uint16_t color) {
  uint32_t i;
  uint16_t w, h;
  w = x2 - x1;
  h = y2 - y1;
  uint16_t total = w * h;
  for (i = 0; i < total; i++)
    lcd_buff[i] = color;
  _spi_mode_set();
  _lcd_set_address(x1, y1, x2, y2);
  LCD_DATA;
  LCD_CS_L;
  HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)lcd_buff, total << 1);
  while (HAL_DMA_GetState(hspi1.hdmatx) != HAL_DMA_STATE_READY)
    ;
  LCD_CS_H;
}

void LCD_Driver::draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2,
                                uint16_t y2) {
  _spi_mode_set();
  _lcd_set_address(x1, y1, x2 - 1, y2 - 1);
  LCD_DATA;
  LCD_CS_L;
  HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)lcd_buff, (x2 - x1) * (y2 - y1) * 2);
  while (HAL_DMA_GetState(hspi1.hdmatx) != HAL_DMA_STATE_READY)
    ;
  LCD_CS_H;
}
void LCD_Driver::draw_rectangle_Aim(uint16_t x1, uint16_t y1, uint16_t x2,
                                    uint16_t y2, uint16_t *buff) {
  _spi_mode_set();
  _lcd_set_address(x1, y1, x2 - 1, y2 - 1);
  LCD_DATA;
  LCD_CS_L;
  HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)buff, (x2 - x1) * (y2 - y1) * 2);
}

//
void LCD_Driver::draw_point(uint16_t x, uint16_t y, uint16_t color) {
  // _spi_mode_set();
  _lcd_set_address(x, y, x, y);
  LCD_DATA;
  LCD_CS_L;
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&color, 2, 0xffff);
  LCD_CS_H;
}

void LCD_Driver::draw_showchar(uint16_t x, uint16_t y, uint8_t chr,
                               uint8_t size, uint16_t color, uint16_t bgcolor) {
  uint8_t temp, t1, t;
  uint16_t y0 = y;
  uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
  chr = chr - ' ';
  for (t = 0; t < csize; t++) {
    if (size == 12)
      temp = asc2_1206[chr][t];
    else if (size == 32)
      temp = asc2_3216[chr][t];
    for (t1 = 0; t1 < 8; t1++) {
      if (temp & 0x80)
        draw_point(x, y, color);
      else
        draw_point(x, y, bgcolor);
      temp <<= 1;
      y++;
      if ((y - y0) == size) {
        y = y0;
        x++;
        break;
      }
    }
  }
}

void LCD_Driver::draw_showstring(uint16_t x, uint16_t y, uint8_t *str,
                                 uint8_t size, uint16_t width, uint16_t height,
                                 uint16_t color, uint16_t bgcolor) {
  uint8_t x0 = x;
  width += x;
  height += y;

  while ((*str <= '~') && (*str >= ' ')) // 判断是不是非法字符!
  {
    if (x >= width) {
      x = x0;
      y += size;
    }

    if (y >= height)
      break; // 退出
    draw_showchar(x, y, *str, size, color, bgcolor);
    x += size / 2;
    str++; // 下一个字符
  }
}

uint32_t LCD_Pow(uint8_t m, uint8_t n) {
  uint32_t result = 1;

  while (n--)
    result *= m;

  return result;
}

void LCD_Driver::draw_shownum(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                              uint8_t size, uint16_t color, uint16_t bgcolor) {

  uint8_t t, temp;
  uint8_t enshow = 0;
  for (t = 0; t < len; t++) {

    temp = (num / LCD_Pow(10, len - t - 1)) % 10;
    if (enshow == 0 && t < (len - 1)) {
      if (temp == 0) {
        draw_showchar(x + (size / 2) * t, y, ' ', size, color, bgcolor);
        continue;
      } else
        enshow = 1;
    }

    draw_showchar(x + (size / 2) * t, y, temp + '0', size, color, bgcolor);
  }
}

//
// void LCD_Driver::Color_bar_display(uint8_t len,uint16_t *buff){
//    uint16_t i,j;
//    uint16_t p = lcd_w / len;
//    //uint16_t color[5]= {0xe00a,0x0bcd,0xe234,0xed12,0xccdd};
//    for ( i = 0; i < lcd_h ; i++){
//        for ( j = 0; j < lcd_w ; j++) {
//            lcd_buff[lcd_w * i + j] = buff[ j / p];
//        }
//    }
//}
// void LCD_Driver::update(){
//    _lcd_set_address(0,0,lcd_w - 1,lcd_h - 1);
//    LCD_DATA;
//    LCD_CS_L;
//    HAL_SPI_Transmit_DMA(&hspi1,(uint8_t *)lcd_buff,45040);
//    while(HAL_DMA_GetState(hspi1.hdmatx) != HAL_DMA_STATE_READY);
//    HAL_SPI_Transmit_DMA(&hspi1,(uint8_t *)(lcd_buff +22520),65040);
//    while(HAL_DMA_GetState(hspi1.hdmatx) != HAL_DMA_STATE_READY);
//    LCD_CS_H;
//}
//
// void LCD_Driver::part_update(uint16_t y1,uint16_t y2){
//    uint16_t h = y2 - y1 - 1;
//    while(HAL_DMA_GetState(hspi1.hdmatx) != HAL_DMA_STATE_READY);
//    _lcd_set_address(0,y1,lcd_w - 1,y2 - 1);
//    LCD_DATA;
//    LCD_CS_L;
//    HAL_SPI_Transmit_DMA(&hspi1,(uint8_t *)(lcd_buff + lcd_w*y1),lcd_w*h*2);
//    while(HAL_DMA_GetState(hspi1.hdmatx) != HAL_DMA_STATE_READY);
//    LCD_CS_H;
//}