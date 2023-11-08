#include <stdio.h>
#include <stdint.h>
#include "ds1307.h"
#include "debug_uart.h"


int main(void)
{
  DebugUART_Init();


  if (ds1307_init() == ERROR)
  {
	printf("Failed to initialise the DS1307 chip, CH=1\n\r");
	while(1);
  }

  return 0;

}
