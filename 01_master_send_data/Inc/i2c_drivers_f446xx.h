#ifndef I2C_DRIVERS_F446XX_H_
#define I2C_DRIVERS_F446XX_H_

#include "stm32f4xx.h"

#define HSI_SOURCE                  0U
#define HSE_SOURCE                  1U
#define PLL_SOURCE                  2U

#define SLAVE_ADDRESS               0x68
#define MY_ADDR                     0x64

#define SPEED_STANDARD
// OR
//#define SPEED_FAST
//#define DUTY_ZERO

#define I2C_SCL_SPEED_SM            100000U
#define I2C_SCL_SPEED_FM2K          200000U
#define I2C_SCL_SPEED_FM4K          400000U

void I2C_Inits(I2C_TypeDef *i2cx);
void I2C_MasterSendData(I2C_TypeDef *i2cx, uint8_t *tx_buffer, uint32_t len, uint8_t slave_addr);

uint8_t GPIO_ReadFromInputPin(GPIO_TypeDef *gpiox, uint8_t pin_number);

#endif /* I2C_DRIVERS_F446XX_H_ */
