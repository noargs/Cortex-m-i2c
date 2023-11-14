#include <stm32f446xx_usart_driver.h>

static uint16_t USART_compute_baudrate(uint32_t pclk, uint32_t desired_baud);

void USART_Init(usart_handle_t *usart_handle)
{
  // Enable clock access to USARTx
  USART_PCLK_EN(usart_handle->usartx);

  // Disable USART module
  usart_handle->usartx->CR1 &= ~USART_CR1_UE;

  // Set UART Baud rate
//  usart_handle->usartx->BRR = usart_handle->usart_config.usart_baud_rate;
  usart_handle->usartx->BRR = USART_compute_baudrate(USART_SYSTEM_CLOCK_FREQ, USART_DESIRED_BAUD_RATE);

  // Set transfer direction, want either or both Tx and Rx
  usart_handle->usartx->CR1 = USART_CR1_TE | USART_CR1_RE;

  // Enable USART2 Interrupt in NVIC

  // Enable the UART module
  usart_handle->usartx->CR1 |= USART_CR1_UE;
}

void USART_SendData(USART_TypeDef *usartx, int ch)
{
  while ((usartx->SR & USART_SR_TXE) == RESET);

  usartx->DR = (uint8_t)(ch & 0xFF);
}

static uint16_t USART_compute_baudrate(uint32_t pclk, uint32_t desired_baud)
{
  return ((pclk + (desired_baud/2U))/desired_baud);
}
