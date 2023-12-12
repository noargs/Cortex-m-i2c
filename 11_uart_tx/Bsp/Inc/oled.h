#ifndef INC_OLED_H_
#define INC_OLED_H_

// OLED SSD1306 128x64

#include "stm32f4xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_spi_driver.h"
#include "oled_fonts.h"

#define OLED_SPI_GPIO_PORT            GPIOB
#define OLED_SPI_NSS_GPIO_PIN         GPIO_PIN_12              // CS
#define OLED_SPI_SCK_GPIO_PIN         GPIO_PIN_13              // D0
#define OLED_SPI_MOSI_GPIO_PIN        GPIO_PIN_15              // D1
#define OLED_DATA_COMMAND_GPIO_PIN    GPIO_PIN_5               // DC
#define OLED_RESET_PIN                GPIO_PIN_6               // RES
#define OLED_SPI_GPIO_PIN_SPEED       GPIO_SPEED_FAST
#define OLED_SPI_GPIO_PIN_MODE        GPIO_MODE_ALTFN
#define OLED_SPI_GPIO_OPTYPE          GPIO_OP_TYPE_PP
#define OLED_SPI_GPIO_PUPD            GPIO_PUPD_NO
#define OLED_SPI_GPIO_ALT_FUN         5

#define OLED_SPI                      SPI2
#define OLED_SPI_BUS_CONFIG           SPI_BUS_CONFIG_FD
#define OLED_SPI_DEVICE_MODE          SPI_DEVICE_MODE_MASTER
#define OLED_SPI_SCLK_SPEED           SPI_SCLK_SPEED_DIV2
#define OLED_SPI_DFF                  SPI_DFF_8BITS
#define OLED_SPI_CPHA                 SPI_CPHA_HIGH            // trailing edge RM page 17 Figure 8-5
#define OLED_SPI_CPOL                 SPI_CPOL_HIGH            // rising edge RM page 17 Figure 8-5
#define OLED_SPI_SSM                  SPI_SSM_DISABLE          // Default case (Hardware slave management is ON)

#define OLED_SSD1306_BUFFER_SIZE      ((OLED_SSD1306_WIDTH * OLED_SSD1306_HEIGHT)/8)
#define OLED_SSD1306_WIDTH            128
#define OLED_SSD1306_HEIGHT           64

#define OLED_X_OFFSET_LOWER           0
#define OLED_X_OFFSET_UPPER           0

#define OFF                           0
#define ON                            1

typedef enum
{
  black = 0x00, // no pixel
  white = 0x01  // inherit oled colors
}oled_ssd1306_color_t;

typedef struct
{
  uint16_t current_x;
  uint16_t current_y;
  uint8_t  initialised;
  uint8_t  display_on;
}oled_ssd1306_t;


// SPI2 peripheral
//
// [NUCLEO]   [SPI]      [OLED - SSD1306]
//   PB12 =>   NSS  =>     CS
//   PB13 =>   SCK  =>     D0
//   PB15 =>   MOSI =>     D1
//   PB5  =>               DC
//   PB6  =>               RES

void oled_init(void);
void oled_display(uint8_t on);
void oled_fill(oled_ssd1306_color_t color);
void oled_update_view(void);

void oled_set_cursor(uint8_t x, uint8_t y);

char oled_write_char(char ch, oled_font_t font, oled_ssd1306_color_t color);
char oled_write_str(char* str, oled_font_t font, oled_ssd1306_color_t color);

void test_font(void);

#endif /* INC_OLED_H_ */
