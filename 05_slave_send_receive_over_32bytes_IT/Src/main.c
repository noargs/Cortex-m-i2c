#include <stdint.h>
#include <string.h>
#include "i2c_drivers_f446xx.h"

//     -- [ I2C Slave (STM32) and I2C Master (Arduino board) communication ] --
//  ... Slave send more than 32 bytes (Arduino wire lib workaround) Interrupt ...
//
// Run the STM32 code in the Discovery board, open Arduino serial monitor Tools > Serial Monitor
// Type the character `s` in the Serial monitor field and hit Send
//
// First Master (Arduino) has to get the length of the data from the Slave (STM32) to read
//   subsequent data from the Slave
// Master (Arduino), should read and display data from (STM32) Slave (Slave transmission / Request for Data)
//
// 1. Use I2C SCL = 100KHz (SM)
// 2. Use external pull up resistors (3.3kOhm) for SDA and SCL line
// Note: if you don't have external pull up, you can try STM32 I2C pin's internal pull up resistors
//
// Arduino sketch (i2x_master_rx_string.ino)

#define GPIOBEN               (0x1UL << (1U))

i2c_handle_t i2c1_handle;


uint8_t tx_buffer[] = "Before a computer program can be executed, it must be created and compiled. Any text editor or IDE is used to edit the Java source code. The extension of the file should be.java, while the file name should reflect the term used in the public class line. The next step when creating a program is to compile the code: the JDK package must be installed on the machine. The compiler conve*** glitch...123";
uint32_t data_length = 0;
uint8_t command_code;

// APB1
// PB8       I2C1_SCL
// PB9       I2C1_SDA

// PC2       External button (pull-down activated), other side of the button to Vdd

void I2C1_GPIOInits();
void GPIO_ButtonInit(void);
void delay(void);

int main(void)
{
  data_length = strlen((char*)tx_buffer);

  i2c1_handle.i2cx = I2C1;

  I2C1_GPIOInits();
  I2C_Inits(i2c1_handle.i2cx);

  // I2C IRQ configurations for Event and Error
  I2C_IRQInterruptConfig(I2C1_EV_IRQn, ENABLE);
  I2C_IRQInterruptConfig(I2C1_ER_IRQn, ENABLE);

  //It has to be enabled to get the interrupt as in master mode it was enabled in the Driver layer
  // Enable ITBUFEN control bit
  i2c1_handle.i2cx->CR2 |= I2C_CR2_ITBUFEN;
  // Enable ITEVTEN control bit
  i2c1_handle.i2cx->CR2 |= I2C_CR2_ITEVTEN;
  // Enable ITERREN control bit
  i2c1_handle.i2cx->CR2 |= I2C_CR2_ITERREN;

  // Enable the I2C peripheral only when you configured it already
  i2c1_handle.i2cx->CR1 |= I2C_CR1_PE;

  // PE=1 then only you can enable ACK [Reference Manual page:861]
  // If PE=0 then ACK cannot be enabled (i.e. 1)
  i2c1_handle.i2cx->CR1 |= I2C_CR1_ACK;

  while (1);

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
  static uint8_t count = 0;
  static uint32_t w_ptr = 0;

  if (APPLICATION_EVENT == I2C_ERROR_AF)
  {
	// Runs only in Slave Tx (end of Slave transmission),
	// Slave should understand Master needs no more data
	// If the current code is 0x52 then don't invalidate
	if (! (command_code == 0x52))
	{
	  command_code = 0xff;
	  count = 0;
	}

	if (w_ptr >= (data_length))
	{
	  w_ptr = 0;
	  command_code = 0xff;
	}
  }
  else if (APPLICATION_EVENT == I2C_EV_STOP)
  {
	// Runs only in Slave Rx (end of Slave reception)
	count = 0;
  }
  else if (APPLICATION_EVENT == I2C_EV_DATA_REQUEST)
  {
	// Master is requesting data, send data
	if (command_code == 0x51)
	{
	  // here sending 4 bytes of length information
	  I2C_SlaveSendData(i2c1_handle.i2cx, ((data_length >> (count % 4) * 8)) & 0xff);
	  count++;
	}
	else if (command_code == 0x52)
	{
	  // Sending `tx_buffer`'s contents indexed by w_ptr variable
	  I2C_SlaveSendData(i2c1_handle.i2cx, tx_buffer[w_ptr++]);
	}

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
