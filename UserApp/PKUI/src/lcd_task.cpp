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

  while (1) {
    lcd_dwt_start = DWT->CYCCNT;
    char_num++;
    if (char_num > 126)
      char_num = 32;

    lcd_dwt_end = DWT->CYCCNT;
    osDelay(1);
  }
}
