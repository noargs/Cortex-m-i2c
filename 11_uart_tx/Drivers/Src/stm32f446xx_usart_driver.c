#include <stm32f446xx_usart_driver.h>

static uint16_t USART_compute_baudrate(uint32_t pclk, uint32_t desired_baud);

void USART_Init(usart_handle_t *usart_handle)
{
  uint32_t temp_reg =0;

  USART_PCLK_EN(usart_handle->usartx);

  switch (usart_handle->usart_config.usart_mode)
  {
  case USART_MODE_RX_ONLY: temp_reg |= USART_CR1_RE; break;
  case USART_MODE_TX_ONLY: temp_reg |= USART_CR1_TE; break;
  default:                 temp_reg |= USART_CR1_RE | USART_CR1_TE;
  }

  temp_reg |= usart_handle->usart_config.usart_word_length << USART_CR1_M_Pos;

  switch (usart_handle->usart_config.usart_parity_control)
  {
  case USART_PARITY_EN_EVEN: temp_reg |= USART_CR1_PCE; break;
  case USART_PARITY_EN_ODD:  temp_reg |= USART_CR1_PS | USART_CR1_PCE; break;
  default:                   temp_reg &= ~USART_CR1_PCE;
  }

  // --[Configure CR1]--
  usart_handle->usartx->CR1 = temp_reg;

  temp_reg =0;
  temp_reg |= usart_handle->usart_config.usart_no_of_stop_bits << USART_CR2_STOP_Pos;

  // --[Configure CR2]--
  usart_handle->usartx->CR2 = temp_reg;

  temp_reg =0;

  switch (usart_handle->usart_config.usart_hw_flow_control)
  {
  case USART_HW_FLOW_CONTROL_CTS:     temp_reg |= USART_CR3_CTSE; break;
  case USART_HW_FLOW_CONTROL_RTS:     temp_reg |= USART_CR3_RTSE; break;
  case USART_HW_FLOW_CONTROL_CTS_RTS: temp_reg |= USART_CR3_RTSE | USART_CR3_CTSE; break;
  default: /* give me a */ break;
  }

  // --[Configure CR3]--
  usart_handle->usartx->CR3 = temp_reg;

}

void USART_SendData(usart_handle_t* usart_handle, uint8_t* tx_buffer, uint32_t length)
{
  uint16_t *data;

  for (uint32_t i=0; i<length; i++)
  {
	while ((usart_handle->usartx->SR & USART_SR_TXE) == RESET);

	if (usart_handle->usart_config.usart_word_length == USART_WORDLEN_9BITS)
	{
	  data = (uint16_t*)tx_buffer;
	  usart_handle->usartx->DR = (*data & (uint16_t)0x01FF);

	  if (usart_handle->usart_config.usart_parity_control == USART_PARITY_DISABLE)
	  {
		tx_buffer++;
		tx_buffer++;
	  }
	  else
	  {
		tx_buffer++;
	  }
	}
	else
	{
	  usart_handle->usartx->DR = (*tx_buffer & (uint8_t)0xFF);
	  tx_buffer++;
	}
  }

  while ((usart_handle->usartx->SR & USART_SR_TC) == RESET); // wait for transmission to complete
}

void USART_ReceiveData(usart_handle_t* usart_handle, uint8_t* rx_buffer, uint32_t length)
{
  for (uint32_t i=0; i<length; i++)
  {
	while ((usart_handle->usartx->SR & USART_SR_RXNE) == RESET);

	if (usart_handle->usart_config.usart_word_length == USART_WORDLEN_9BITS)
	{
	  if (usart_handle->usart_config.usart_parity_control == USART_PARITY_DISABLE)
	  {
		*((uint16_t*)rx_buffer) = (usart_handle->usartx->DR & (uint16_t)0x01FF);
		rx_buffer++;
		rx_buffer++;
	  }
	  else
	  {
		*rx_buffer = (usart_handle->usartx->DR & (uint8_t)0xFF);
		rx_buffer++;
	  }
	}
	else
	{
	  if (usart_handle->usart_config.usart_parity_control == USART_PARITY_DISABLE)
	  {
		*rx_buffer = (uint8_t)(usart_handle->usartx->DR & (uint8_t)0xFF);
	  }
	  else
	  {
		*rx_buffer = (uint8_t)(usart_handle->usartx->DR & (uint8_t)0x7F);
	  }
	  rx_buffer++;
	}
  }
}

void USART_SendChar(USART_TypeDef *usartx, int ch)
{
  while ((usartx->SR & USART_SR_TXE) == RESET);
  usartx->DR = (uint8_t)(ch & 0xFF);
}

static uint16_t USART_compute_baudrate(uint32_t pclk, uint32_t desired_baud)
{
  return ((pclk + (desired_baud/2U))/desired_baud);
}
