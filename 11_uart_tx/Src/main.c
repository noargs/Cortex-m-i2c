#include <stdio.h>
#include <string.h>
#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_gpio_driver.h"

//     -- [ UART Tx (STM32) and Rx (Arduino board) communication ] --
//
// Send some message `msg` over UART from ST to Arduino. Arduino will display the message
//  on Serial monitor
// External Button PC2 (Pull-down activated), other side of the button connected to Vdd (3v),
//  on STM32 (PC2) button press, STM32 send data to Arduino
//
// Baudrate: 115200 bps
// Frame format: 1 stop bits, 8 bits, no parity
//
// STM32 PA2(Tx) -> LV1 (Logic Analyser CH0) -> HV1 -> ARDUINO 0(Rx)
//       PA3(Rx) -> LV2 (Logic Analyser CH1) -> HV2 -> ARDUINO 1(Tx)
//
// STM32 GND - Voltage converter GND - ARDUINO GND - Logic Analyser GND
// STM32 3V  -> LV -> HV -> STM32 5V

void gpio_button_init(void);
void usart2_gpio_init(void);
void usart2_init (void);
void delay(void);

usart_handle_t usart2_handle;

char msg[1024] = "-[UART Tx testing] ...\n\r";

int main(void)
{
  gpio_button_init();
  usart2_gpio_init();
  usart2_init();
  USART_PeripheralControl(usart2_handle.usartx, ENABLE);

  while (1)
  {
	while (! GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_2)); // button press pull-low
	delay();
	USART_SendData(&usart2_handle, (uint8_t*)msg, strlen(msg));
  }
  return 0;

}

void usart2_init (void)
{
  usart2_handle.usartx                             = USART2;
  usart2_handle.usart_config.usart_baudrate        = USART_STD_BAUD_115200;
  usart2_handle.usart_config.usart_hw_flow_control = USART_HW_FLOW_CONTROL_NONE;
  usart2_handle.usart_config.usart_mode            = USART_MODE_TX_ONLY;
  usart2_handle.usart_config.usart_no_of_stop_bits = USART_STOP_BITS_1;
  usart2_handle.usart_config.usart_parity_control  = USART_PARITY_DISABLE;
  usart2_handle.usart_config.usart_word_length     = USART_WORDLEN_8BITS;
  USART_Init(&usart2_handle);
}

void usart2_gpio_init (void)
{
  gpio_handle_t usart_gpios;
  usart_gpios.gpiox                                = GPIOA;
  usart_gpios.gpio_config.pin_alt_fun              = 7;
  usart_gpios.gpio_config.pin_mode                 = GPIO_MODE_ALTFN;
  usart_gpios.gpio_config.pin_op_type              = GPIO_OP_TYPE_PP;
  usart_gpios.gpio_config.pin_pu_pd                = GPIO_PUPD_PU;
  usart_gpios.gpio_config.pin_speed                = GPIO_SPEED_FAST;

  // USART2 TX
  usart_gpios.gpio_config.pin_number               = GPIO_PIN_2;
  GPIO_Init(&usart_gpios);

  // USART2 RX
  usart_gpios.gpio_config.pin_number               = GPIO_PIN_3;
}

void gpio_button_init(void)
{
  gpio_handle_t button;
  button.gpiox                                     = GPIOC;
  button.gpio_config.pin_mode                      = GPIO_MODE_IN;
  button.gpio_config.pin_op_type                   = GPIO_OP_TYPE_PP;
  button.gpio_config.pin_pu_pd                     = GPIO_PUPD_PD;
  button.gpio_config.pin_speed                     = GPIO_SPEED_FAST;

  button.gpio_config.pin_number = GPIO_PIN_2;
  GPIO_Init(&button);
}

void delay(void)
{
  for (uint32_t i=0; i < 250000; i++);
}
