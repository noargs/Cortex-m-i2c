#include "debug_uart.h"
#include <string.h>

static void DebugUART_gpio_config(void);
static void DebugUART_uart_config(void);
static void DebugUART_write(int ch);

usart_handle_t usart2_handle;

int __io_putchar(int ch)
{
  DebugUART_write(ch);
  return ch;
}

void DebugUART_Init(void)
{
  DebugUART_gpio_config();
  DebugUART_uart_config();

}

static void DebugUART_gpio_config(void)
{
// APB1
// PA2   ==> USART2_TX (AF07)
// PA3   ==> USART2_RX (AF07)

  gpio_handle_t usart2_tx, usart2_rx;

//  memset(&usart2_tx, 0, sizeof(usart2_tx));
//  memset(&usart2_rx, 0, sizeof(usart2_rx));

  usart2_tx.gpiox                   = DEBUG_UART_GPIO_PORT;
  usart2_tx.gpio_config.pin_alt_fun = AF7;
  usart2_tx.gpio_config.pin_mode    = DEBUG_UART_GPIO_PIN_MODE;
  usart2_tx.gpio_config.pin_number  = DEBUG_UART_TX_GPIO_PIN;
  usart2_tx.gpio_config.pin_speed   = DEBUG_UART_GPIO_PIN_SPEED;
  GPIO_Init(&usart2_tx);

  usart2_rx.gpiox                   = DEBUG_UART_GPIO_PORT;
  usart2_rx.gpio_config.pin_alt_fun = AF7;
  usart2_rx.gpio_config.pin_mode    = DEBUG_UART_GPIO_PIN_MODE;
  usart2_rx.gpio_config.pin_number  = DEBUG_UART_RX_GPIO_PIN;
  usart2_rx.gpio_config.pin_speed   = DEBUG_UART_GPIO_PIN_SPEED;
  GPIO_Init(&usart2_rx);
}

static void DebugUART_uart_config(void)
{
  usart2_handle.usartx = USART2;
  USART_Init(&usart2_handle);
}

static void DebugUART_write(int ch)
{
  USART_SendData(usart2_handle.usartx, ch);
}
