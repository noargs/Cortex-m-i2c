#include "lcd.h"


void lcd_init(void)
{
  gpio_handle_t lcd;

  lcd.gpiox                    = LCD_GPIO_PORT;
  lcd.gpio_config.pin_mode     = GPIO_MODE_OUT;
  lcd.gpio_config.pin_op_type  = GPIO_OP_TYPE_PP;
  lcd.gpio_config.pin_pu_pd    = GPIO_PUPD_NO;
  lcd.gpio_config.pin_speed    = GPIO_SPEED_FAST;

  lcd.gpio_config.pin_number   = LCD_GPIO_RS;
  GPIO_Init(&lcd);

  lcd.gpio_config.pin_number   = LCD_GPIO_RW;
  GPIO_Init(&lcd);

  lcd.gpio_config.pin_number   = LCD_GPIO_EN;
  GPIO_Init(&lcd);

  lcd.gpio_config.pin_number   = LCD_GPIO_D4;
  GPIO_Init(&lcd);

  lcd.gpio_config.pin_number   = LCD_GPIO_D5;
  GPIO_Init(&lcd);

  lcd.gpio_config.pin_number   = LCD_GPIO_D6;
  GPIO_Init(&lcd);

  lcd.gpio_config.pin_number   = LCD_GPIO_D7;
  GPIO_Init(&lcd);

}
