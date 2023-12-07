#include <stdio.h>
#include "lm75b.h"
#include "debug_uart.h"


int main(void)
{
  DebugUART_Init();
  lm75b_init();

  float temp;
  lm75b_get_temperature(&temp);

  printf(" Room temp: %0.1f C\n",temp);

  while(1);
  return 0;
}
