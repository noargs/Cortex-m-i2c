#include <stdint.h>
#include <string.h>
#include "i2c_drivers_f446xx.h"

//     -- [ I2C Slave (STM32) and I2C Master (Arduino board) communication ] --
//                              ... Interrupt ...
//
// Master (Arduino), should read and display data from (STM32) Slave
// First Master (Arduino) has to get the length of the data from the Slave (STM32) to read
//   subsequent data from the Slave
//
// 1. Use I2C SCL = 100KHz (SM)
// 2. Use external pull up resistors (3.3kOhm) for SDA and SCL line
// Note: if you don't have external pull up, you can try STM32 I2C pin's internal pull up resistors

// Arduino sketch (i2x_master_rx_string.ino)

#define GPIOBEN               (0x1UL << (1U))

uint8_t tx_buffer[32] = "STM32 Slave mode testing";
i2c_handle_t i2c1_handle;

// APB1
// PB8       I2C1_SCL
// PB9       I2C1_SDA

// PC2       External button (pull-down activated), other side of the button to Vdd

void I2C1_GPIOInits();
void GPIO_ButtonInit(void);
void delay(void);

int main(void)
{
  uint8_t command_code;
  uint8_t len;

  i2c1_handle.i2cx = I2C1;

  I2C1_GPIOInits();
  I2C_Inits(i2c1_handle.i2cx);

  // I2C IRQ configurations for Event and Error
  I2C_IRQInterruptConfig(I2C1_EV_IRQn, ENABLE);
  I2C_IRQInterruptConfig(I2C1_ER_IRQn, ENABLE);

  // Enable the I2C peripheral only when you configured it already
  i2c1_handle.i2cx->CR1 |= I2C_CR1_PE;

  // PE=1 then only you can enable ACK [Reference Manual page:861]
  // If PE=0 then ACK cannot be enabled (i.e. 1)
  i2c1_handle.i2cx->CR1 |= I2C_CR1_ACK;

}

void I2C1_EV_IRQHandler (void)
{
  I2C_EV_IRQHandling(&i2c1_handle);
}

void I2C1_ER_IRQHandler (void)
{
  I2C_ERR_IRQHandling(&i2c1_handle);
}

void I2C_ApplicationEventCallback(i2c_handle_t *i2c_handle, uint8_t APPLICATION_EVENT)
{
  if (APPLICATION_EVENT == I2C_EV_TX_COMPLETE)
  {
//	printf("Tx is completed\n");
  }
  else if (APPLICATION_EVENT == I2C_EV_RX_COMPLETE)
  {
//	printf("Rx is compeleted\n");
  }
  else if (APPLICATION_EVENT == I2C_ERROR_AF)
  {
//	printf("Error: ACK Failure\n");

	I2C_CloseSendData(&i2c1_handle);

	i2c1_handle.i2cx->CR1 |= I2C_CR1_STOP;

	while(1);
  }
}

void I2C1_GPIOInits()
{
  // Enable clock access
  RCC->AHB1ENR |= GPIOBEN;

  // PB8 and PB9 to AF mode
  GPIOB->MODER &= ~(0x3UL << (16U));
  GPIOB->MODER |= (0x2UL << (16U));

  GPIOB->MODER &= ~(0x3UL << (18U));
  GPIOB->MODER |= (0x2UL << (18U));

  // PB8 and PB9 to AF4
  GPIOB->AFR[1] &= ~(0xFUL << (0U));
  GPIOB->AFR[1] |= (0x4UL << (0U));

  GPIOB->AFR[1] &= ~(0xFUL << (4U));
  GPIOB->AFR[1] |= (0x4UL << (4U));

  // PB8 and PB9 OTYPE (open-drain)
  GPIOB->OTYPER |= (0x1UL << (8U));
  GPIOB->OTYPER |= (0x1UL << (9U));

  // PB8 and PB9 (No pull-up, pull-down)
  GPIOB->PUPDR &= ~(0x3UL << (16U));
  GPIOB->PUPDR &= ~(0x3UL << (18U));

  // PB8 and PB9 OSPEED (Fast speed)
  GPIOB->OSPEEDR &= ~(0x3UL << (16U));
  GPIOB->OSPEEDR |= (0x2UL << (16U));

  GPIOB->OSPEEDR &= ~(0x3UL << (18U));
  GPIOB->OSPEEDR |= (0x2UL << (18U));
}

void delay(void)
{
  for (uint32_t i=0; i < 500000/2; i++);
}
