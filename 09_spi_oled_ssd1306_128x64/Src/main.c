#include <stdio.h>
#include <stdint.h>
#include "lm75b.h"
#include "oled.h"
#include "debug_uart.h"


void mdelay(uint32_t count)
{
  for (uint32_t i=0; i< (count * 1000); i++);
}

int main(void)
{
  DebugUART_Init();

//  float temp;
//  lm75b_init();
//  lm75b_basic_temp((float*)&temp);
//  printf("Room temp: %0.1f C\n", temp);

  oled_init();

  oled_fill(white);

  oled_update_view();

  mdelay(1000);

  test_font();

  while(1);
  return 0;
}
