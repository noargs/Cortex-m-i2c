#include <stdint.h>
#include "stm32f446xx_i2c_driver.h"


#define GPIOBEN                    (0x1UL << (1U))

int main(void)
{

  I2C1_PCLK_EN();

  return 0;

}
