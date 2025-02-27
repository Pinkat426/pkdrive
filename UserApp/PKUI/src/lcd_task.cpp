#include "lcd_task.h"
#include "lcd_driver.h"
#include "stm32f4xx_hal_gpio.h"
#include <cstdint>

uint32_t lcd_dwt_start;
uint32_t lcd_dwt_end;

void lcd_task(void) {
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
  LCD_Driver lcd;
  lcd.Init(3);
  lcd.fill(0x0000);
  // lcd.draw_rectangle(0, 0, 172, 320);
  uint16_t char_num = 32;
  lcd.draw_showchar(10, 10, 'a', 32, 0xffff, 0x0000);
  uint8_t run_time = 0;
  while (1) {
    lcd_dwt_start = DWT->CYCCNT;
    // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
    // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);

    char_num++;
    if (char_num > 126)
      char_num = 32;
    lcd.draw_showchar(10, 10, char_num, 32, 0xffff, 0x0000);
    //  lcd.draw_showstring(10, 50, (uint8_t *)"Hello World", 32, 172, 320,
    //  0xffff,
    //                      0x0000);
    run_time++;
    // lcd.draw_shownum(10, 100, run_time, 3, 32, 0xffff, 0x0000);
    lcd_dwt_end = DWT->CYCCNT;
    // PRINT(lcd, "%d", lcd_dwt_end - lcd_dwt_start);
    if (run_time >= 998)
      run_time = 0;
    osDelay(1);
  }
}
