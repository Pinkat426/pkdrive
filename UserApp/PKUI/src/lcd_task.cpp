#include "lcd_task.h"
#include "encoderkey_driver.h"
#include "lcd_driver.h"
#include "stm32f4xx_hal_gpio.h"
#include <cstdint>

EncoderKey_Driver encoderkey_driver;
uint32_t lcd_dwt_start;
uint32_t lcd_dwt_end;

void lcd_task(void) {
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
  LCD_Driver lcd;
  lcd.Init(3);
  lcd.fill(0x0000);
  uint16_t char_num = 32;
  // lcd.draw_showchar(10, 10, 'a', 32, 0xffff, 0x0000);

  while (1) {
    lcd_dwt_start = DWT->CYCCNT;
    char_num++;
    if (char_num > 126)
      char_num = 32;
    // lcd.draw_showchar(10, 10, char_num, 32, 0xffff, 0x0000);
    lcd_dwt_end = DWT->CYCCNT;
    encoderkey_driver.detect_key_state();

    if (encoderkey_driver.get_key_state() == EncoderKey_Click) {
      lcd.draw_showchar(10, 10, 'a', 32, 0xffff, 0x0000);
      PRINT(lcd_task, "EncoderKey_Click");
    } else if (encoderkey_driver.get_key_state() == EncoderKey_LClick) {
      lcd.draw_showchar(10, 10, 'b', 32, 0xffff, 0x0000);
      PRINT(lcd_task, "EncoderKey_LClick");
    } else if (encoderkey_driver.get_key_state() == EncoderKey_DClick) {
      lcd.draw_showchar(10, 10, 'c', 32, 0xffff, 0x0000);
      PRINT(lcd_task, "EncoderKey_DClick");
    } else if (encoderkey_driver.get_key_state() == EncoderKey_Left) {
      lcd.draw_showchar(10, 10, 'd', 32, 0xffff, 0x0000);
      PRINT(lcd_task, "EncoderKey_Left");
    } else if (encoderkey_driver.get_key_state() == EncoderKey_Right) {
      lcd.draw_showchar(10, 10, 'e', 32, 0xffff, 0x0000);
      PRINT(lcd_task, "EncoderKey_Right");
    }
    osDelay(1);
  }
}
