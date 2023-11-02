#ifndef STM32F446XX_I2C_DRIVER_H_
#define STM32F446XX_I2C_DRIVER_H_

#include "stm32f4xx.h"

typedef struct
{
  uint32_t i2c_scl_speed;
  uint8_t  i2c_device_address;
  uint8_t  i2c_ack;
  uint8_t  i2c_fm_duty_cycle;
}i2c_config_t;

typedef struct
{
  I2C_TypeDef *i2cx;
  i2c_config_t i2c_config;
}i2c_handle_t;

// @i2c_scl_speed
#define I2C_SCL_SPEED_SM                  100000 // Hz
#define I2C_SCL_SPEED_FM2K                200000
#define I2C_SCL_SPEED_FM4K                400000

// @i2c_ack_control
#define I2C_ACK_ENABLE                    SET
#define I2C_ACK_DISABLE                   RESET

// @i2c_fm_duty_cyle
#define I2C_FM_DUTY_2                     0
#define I2C_FM_DUTY_16_9                  1

#define I2C1_PCLK_EN()                    (RCC->APB1ENR |= RCC_APB1ENR_I2C1EN)
#define I2C2_PCLK_EN()                    (RCC->APB1ENR |= RCC_APB1ENR_I2C2EN)
#define I2C3_PCLK_EN()                    (RCC->APB1ENR |= RCC_APB1ENR_I2C3EN)

#define I2C1_PCLK_DI()                    (RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN)
#define I2C2_PCLK_DI()                    (RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN)
#define I2C3_PCLK_DI()                    (RCC->APB1ENR &= ~RCC_APB1ENR_I2C3EN)

void I2C_Init(i2c_handle_t *i2c_handle);



#endif /* STM32F446XX_I2C_DRIVER_H_ */
