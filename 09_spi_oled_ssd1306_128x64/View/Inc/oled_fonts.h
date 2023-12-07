#ifndef INC_OLED_FONTS_H_
#define INC_OLED_FONTS_H_

#include <stdint.h>

#define OLED_FONT_6x8
#define OLED_FONT_7x10
#define OLED_FONT_11x18
#define OLED_FONT_16x26


typedef struct
{
  const uint8_t font_width;
  uint8_t       font_height;
  const uint16_t *data;
} oled_font_t;


#endif /* INC_OLED_FONTS_H_ */
