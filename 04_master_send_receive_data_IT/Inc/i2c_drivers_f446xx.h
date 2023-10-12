#ifndef I2C_DRIVERS_F446XX_H_
#define I2C_DRIVERS_F446XX_H_

#include <stddef.h>
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


#define I2C_ENABLE_SR               ENABLE
#define I2C_DISABLE_SR              DISABLE


#define I2C_SCL_SPEED_SM            100000U
#define I2C_SCL_SPEED_FM2K          200000U
#define I2C_SCL_SPEED_FM4K          400000U

// @i2c_application_states
#define I2C_READY                   0U
#define I2C_BUSY_IN_RX              1U
#define I2C_BUSY_IN_TX              2U

typedef struct
{
  I2C_TypeDef  *i2cx;
  uint8_t      *tx_buffer;
  uint8_t      *rx_buffer;
  uint32_t     tx_length;
  uint32_t     rx_length;
  uint8_t      tx_rx_state;
  uint8_t      device_address;
  uint32_t     rx_size;
  uint8_t      repeated_start_sr;
} i2c_handle_t;

void I2C_Inits (I2C_TypeDef *i2cx);
void I2C_MasterSendData (I2C_TypeDef *i2cx, uint8_t *tx_buffer, uint32_t len, uint8_t slave_addr, uint8_t repeated_start);
void I2C_MasterReceiveData (I2C_TypeDef *i2cx, uint8_t *rx_buffer, uint8_t len, uint8_t slave_addr, uint8_t repeated_start);

uint8_t GPIO_ReadFromInputPin (GPIO_TypeDef *gpiox, uint8_t pin_number);

uint8_t I2C_MasterSendDataInterrupt (i2c_handle_t *i2c_handle, uint8_t *tx_buffer, uint32_t length, uint8_t slave_address, uint8_t repeated_start);
uint8_t I2C_MasterReceiveDataInterrupt (i2c_handle_t *i2c_handle, uint8_t *rx_buffer, uint32_t length, uint8_t slave_address, uint8_t repeated_start);

void I2C_EV_IRQHandling (i2c_handle_t *i2c_handle);
void I2C_ERR_IRQHandling (i2c_handle_t *i2c_handle);

void I2C_IRQInterruptConfig(uint8_t irq_number, uint8_t enable_or_disable);
void I2C_IRQPriorityConfig(uint8_t irq_number, uint32_t irq_priority);

void I2C_ApplicationEventCallback(i2c_handle_t *i2c_handle, uint8_t APPLICATION_EVENT);


// ARM Cortex Mx Processor NVIC ISERx register Addresses
#define NVIC_ISER0                                  ((volatile uint32_t*)0xE000E100)
#define NVIC_ISER1                                  ((volatile uint32_t*)0xE000E104)
#define NVIC_ISER2                                  ((volatile uint32_t*)0xE000E108)
#define NVIC_ISER3                                  ((volatile uint32_t*)0xE000E10C)

// ARM Cortex Mx Processor NVIC ICERx register addresses
#define NVIC_ICER0                                  ((volatile uint32_t*)0xE000E180)
#define NVIC_ICER1                                  ((volatile uint32_t*)0xE000E184)
#define NVIC_ICER2                                  ((volatile uint32_t*)0xE000E188)
#define NVIC_ICER3                                  ((volatile uint32_t*)0xE000E18C)

// ARM Cortex Mx Processor Priority Register IPR Address
#define NVIC_PR_BASE_ADDR                           ((volatile uint32_t*)0xE000E400)

#define NO_PR_BITS_IMPLEMENTED                      4


// I2C application events macros
#define I2C_EV_TX_COMPLETE                0
#define I2C_EV_RX_COMPLETE                1
#define I2C_EV_STOP                       2
#define I2C_ERROR_BERR                    3
#define I2C_ERROR_ARLO                    4
#define I2C_ERROR_AF                      5
#define I2C_ERROR_OVR                     6
#define I2C_ERROR_TIMEOUT                 7
#define I2C_EV_DATA_REQUEST               8
#define I2C_EV_DATA_RECEIVE               9


#endif /* I2C_DRIVERS_F446XX_H_ */
