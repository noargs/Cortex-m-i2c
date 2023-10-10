#include <stdint.h>
#include <string.h>
#include "i2c_drivers_f446xx.h"

//     -- [ I2C Master (STM32) and I2C Slave (Arduino board) communication ] --
//                      ... Repeated Start ...
//
// External Button PC2 (Pull-down activated), connected to Vdd on button other side,
//   on STM32 (Master) is pressed,
// Master should read and display data from Slave (Arduino),
// First master has to get the length of the data from the Slave to read
//   subsequent data from the slave
//
// 1. Use I2C SCL = 100KHz (SM)
// 2. Use external pull up resistors (3.3kOhm) for SDA and SCL line
// Note: if you don't have external pull up, you can try STM32 I2C pin's internal pull up resistors

// Arduino sketch (i2x_slave_tx_string)

#define GPIOCEN               (0x1UL << (2U))
#define GPIOBEN               (0x1UL << (1U))
#define USER_BUTTON_NUCLEO    2U

uint8_t receive_buffer[];
I2C_TypeDef *I2Cx = I2C1;

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

  GPIO_ButtonInit();
  I2C1_GPIOInits();
  I2C_Inits(I2Cx);

  // Enable the I2C peripheral only when you configured it already
  I2Cx->CR1 |= I2C_CR1_PE;

  // PE=1 then only you can enable ACK [Reference Manual page:861]
  // If PE=0 then ACK cannot be enabled (i.e. 1)
  I2Cx->CR1 |= I2C_CR1_ACK;

  while(1)
  {
	while(! GPIO_ReadFromInputPin(GPIOC, USER_BUTTON_NUCLEO));
	delay();

	// `length information`
	command_code = 0x51;
	I2C_MasterSendData(I2Cx, &command_code, 1, SLAVE_ADDRESS, I2C_ENABLE_SR);
	I2C_MasterReceiveData(I2Cx, &len, 1, SLAVE_ADDRESS, I2C_ENABLE_SR);

	// `Actual data with above length`
	command_code = 0x52;
	I2C_MasterSendData(I2Cx, &command_code, 1, SLAVE_ADDRESS, I2C_ENABLE_SR);
	I2C_MasterReceiveData(I2Cx, receive_buffer, len, SLAVE_ADDRESS, I2C_DISABLE_SR);
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

void GPIO_ButtonInit(void)
{
  RCC->AHB1ENR |= GPIOCEN;

  // User button @ GPIOC (PC2) (Input)
  GPIOC->MODER &= ~(0x3UL << (4U));

  // PA0 OTYPE (ouput push-pull)
  //GPIOC->OTYPER &= ~(0x1UL << (2U));

  // PA0 OSPEEDR (Fast speed)
  GPIOC->OSPEEDR &= ~(0x3UL << (4U));
  GPIOC->OSPEEDR |= (0x2UL << (4U));

  // PA0 (No pull-up, pull-down)
  GPIOC->PUPDR &= ~(0x3UL << (4U));
  GPIOC->PUPDR |= (0x2UL << (4U));
}

void delay(void)
{
  for (uint32_t i=0; i < 500000/2; i++);
}
