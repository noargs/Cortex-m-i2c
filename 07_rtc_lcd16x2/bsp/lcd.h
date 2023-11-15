#ifndef LCD_H_
#define LCD_H_

#include "stm32f446xx_gpio_driver.h"

#define LCD_GPIO_PORT                     GPIOA
#define LCD_GPIO_RS                       GPIO_PIN_5
#define LCD_GPIO_RW                       GPIO_PIN_6
#define LCD_GPIO_EN                       GPIO_PIN_7
#define LCD_GPIO_D4                       GPIO_PIN_0
#define LCD_GPIO_D5                       GPIO_PIN_1
#define LCD_GPIO_D6                       GPIO_PIN_2
#define LCD_GPIO_D7                       GPIO_PIN_3

void lcd_init(void);


// Vss     ==>  GND
// Vdd     ==>  3.3v
// V0/VE   ==>  (Contrast, middle pin of 10K potentiometer)
// RS      ==>  PA5
// R/W     ==>  PA6
// En      ==>  PA7
// D4      ==>  PA0
// D5      ==>  PA1
// D6      ==>  PA2
// D7      ==>  PA3
// A       ==>  LED+
// K       ==>  LED-

#endif /* LCD_H_ */
