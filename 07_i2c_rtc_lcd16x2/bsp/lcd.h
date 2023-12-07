#ifndef LCD_H_
#define LCD_H_

#include "stm32f446xx_gpio_driver.h"

#define LCD_GPIO_PORT                     GPIOC
#define LCD_GPIO_RS                       GPIO_PIN_0
#define LCD_GPIO_RW                       GPIO_PIN_1
#define LCD_GPIO_EN                       GPIO_PIN_2
#define LCD_GPIO_D4                       GPIO_PIN_6
#define LCD_GPIO_D5                       GPIO_PIN_8
#define LCD_GPIO_D6                       GPIO_PIN_14
#define LCD_GPIO_D7                       GPIO_PIN_15

// LCD commands
#define LCD_CMD_DL4BIT_N2LINES_F5X8DOTS   0x28         // DL=1 (i.e. 4bits), N=1 (2-lines), F=0 (5x8 dots)
#define LCD_CMD_DISPLAYON_CURSORON        0x0E
#define LCD_CMD_DISPLAY_CLEAR             0x01
#define LCD_CMD_INC_DDRAM                 0x06
#define LCD_CMD_DISPLAY_RETURN_HOME       0x02

void lcd_init(void);
void lcd_send_command(uint8_t command);
void lcd_print_char(uint8_t data);
void lcd_print_string(char *msg);
void lcd_display_clear(void);
void lcd_display_return_home(void);
void lcd_set_cursor(uint8_t row, uint8_t column);
void lcd_mdelay(uint32_t count);

// Vss     ==>  GND
// Vdd     ==>  3.3v
// V0/VE   ==>  (Contrast, middle pin of 10K potentiometer)
// RS      ==>  PC0
// R/W     ==>  PC1 OR GND 0 = write throughout the execution of application
// En      ==>  PC2
// D4      ==>  PC6
// D5      ==>  PC8
// D6      ==>  PC14
// D7      ==>  PC15
// A       ==>  LED+
// K       ==>  LED-

#endif /* LCD_H_ */
