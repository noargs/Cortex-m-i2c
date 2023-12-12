#ifndef STM32F446XX_I2C_DRIVER_H_
#define STM32F446XX_I2C_DRIVER_H_

#include "stm32f4xx.h"
#include "stm32f446xx_gpio_driver.h"

typedef struct
{
  uint32_t i2c_scl_speed;
  uint8_t  i2c_device_address;
  uint8_t  i2c_ack_control;
  uint8_t  i2c_fm_duty_cycle;
}i2c_config_t;

typedef struct
{
  I2C_TypeDef *i2cx;
  i2c_config_t i2c_config;
}i2c_handle_t;


// @i2c_scl_speed
#define I2C_SCL_SPEED_XSM                 25000 // Hz
#define I2C_SCL_SPEED_SM                  100000
#define I2C_SCL_SPEED_FM2K                200000
#define I2C_SCL_SPEED_FM4K                400000

// @i2c_ack_control
#define I2C_ACK_ENABLE                    SET
#define I2C_ACK_DISABLE                   RESET

// @i2c_fm_duty_cyle
#define I2C_FM_DUTY_2                     0
#define I2C_FM_DUTY_16_9                  1

#define I2C_SR_ENABLE                     ENABLE
#define I2C_SR_DISABLE                    DISABLE

#define I2C_PCLK_EN(x)                    ((x == I2C1) ? (RCC->APB1ENR |= RCC_APB1ENR_I2C1EN) :\
                                           (x == I2C2) ? (RCC->APB1ENR |= RCC_APB1ENR_I2C2EN) :\
                                           (x == I2C3) ? (RCC->APB1ENR |= RCC_APB1ENR_I2C3EN) : 0)

#define I2C_PCLK_DIS(x)                   ((x == I2C1) ? (RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN) :\
                                           (x == I2C2) ? (RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN) :\
                                           (x == I2C3) ? (RCC->APB1ENR &= ~RCC_APB1ENR_I2C3EN) : 0)

#define SYSTEM_CLOCK_SWS_HSI              0U
#define SYSTEM_CLOCK_SWS_HSE              1U
#define SYSTEM_CLOCK_SWS_PLL              2U
#define SYSTEM_CLOCK_SWS_PLL_R            3U


void I2C_Init(i2c_handle_t *i2c_handle);

void I2C_MasterSendData(i2c_handle_t *i2c_handle, uint8_t *tx_buffer, uint32_t length, uint8_t slave_address, uint8_t sr_repeated_start );
void I2C_MasterReceiveData(i2c_handle_t *i2c_handle, uint8_t *rx_buffer, uint32_t length, uint8_t slave_address, uint8_t sr_repeated_start);
void I2C_ClearBusyFlagErratum(gpio_handle_t* i2c_scl, gpio_handle_t* i2c_sda, i2c_handle_t *i2c_handle );


#endif /* STM32F446XX_I2C_DRIVER_H_ */
