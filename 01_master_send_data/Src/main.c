#include <stdint.h>
#include <string.h>
#include "i2c_drivers_f446xx.h"

//     -- [ I2C Master (STM32) and I2C Slave (Arduino board) communication ] --
//
// External Button PC2 (Pull-down activated), connected to Vdd on button other side, on STM32 (Master) is pressed, Master send data to Slave (Arduino)
// Data received on Arduino (Slave) will be displayed on Serial Monitor of Arduino IDE
//
// 1. Use I2C SCL = 100KHz (SM)
// 2. Use external pull up resistors (3.3kOhm) for SDA and SCL line
// Note: if you don't have external pull up, you can try STM32 I2C pin's internal pull up resistors

// Arduino sketch (i2x_slave_rx_string)

#define GPIOCEN               (0x1UL << (2U))
#define GPIOBEN               (0x1UL << (1U))
#define USER_BUTTON_NUCLEO    2U

I2C_TypeDef *I2Cx = I2C1;
uint8_t data[] = "https://github.com\n";

// APB1
// PB8       I2C1_SCL
// PB9       I2C1_SDA

void I2C1_GPIOInits();
void GPIO_ButtonInit(void);
void delay(void);

int main(void)
{
  GPIO_ButtonInit();
  I2C1_GPIOInits();
  I2C_Inits(I2Cx);

  // Enable the I2C peripheral only when you configured it already
  I2Cx->CR1 |= I2C_CR1_PE;

  while(1)
  {
	while(! GPIO_ReadFromInputPin(GPIOC, USER_BUTTON_NUCLEO));

	delay();

	I2C_MasterSendData(I2Cx, data, strlen((char*)data), SLAVE_ADDRESS);
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

  // User button @ GPIOC (PC13) (Input)
//  GPIOC->MODER &= ~(0x3UL << (26U));
  GPIOC->MODER &= ~(0x3UL << (4U));

  // PA0 OTYPE (ouput push-pull)
  //GPIOC->OTYPER &= ~(0x1UL << (13U));

  // PA0 OSPEEDR (Fast speed)
//  GPIOC->OSPEEDR &= ~(0x3UL << (26U));
//  GPIOC->OSPEEDR |= (0x2UL << (26U));
  GPIOC->OSPEEDR &= ~(0x3UL << (4U));
  GPIOC->OSPEEDR |= (0x2UL << (4U));

  // PA0 (No pull-up, pull-down)
//  GPIOC->PUPDR &= ~(0x3UL << (26U));
  GPIOC->PUPDR &= ~(0x3UL << (4U));
  GPIOC->PUPDR |= (0x2UL << (4U));
//  GPIOC->PUPDR &= ~(0x3UL << (6U));
//  GPIOC->PUPDR |= (0x1UL << (6U));
}

void delay(void)
{
  for (uint32_t i=0; i < 500000/2; i++);
}
