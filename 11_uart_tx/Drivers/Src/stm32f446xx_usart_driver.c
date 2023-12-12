#include <stm32f446xx_usart_driver.h>
#include <stm32f446xx_rcc_driver.h>

static void USART_SetBaudrate(USART_TypeDef* usartx, uint32_t desired_baudrate);

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

  // --[Configure BRR]--
  USART_SetBaudrate(usart_handle->usartx, usart_handle->usart_config.usart_baudrate);

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

static void USART_SetBaudrate(USART_TypeDef* usartx, uint32_t desired_baudrate)
{
  uint32_t pclk, usart_divider, mantissa, fraction, temp_reg =0;

  if (usartx == USART1 || usartx == USART6)
	pclk = RCC_GetPCLK2Value();
  else
	pclk = RCC_GetPCLK1Value();


  //                       FPCLK                                FPCLK x 25                  400000000
  // usart_divider = ------------------  x 100     =>       -------------------    =>      ------------  =  1736.111
  //                   8 (or 16) x B.R                         2 (or 4) x B.R                230400
  //
  // Actual formula is [Fpclk / 8 x B.R], However multiply with 100 (as done above) so we can also divide with 100
  //  to get the mantissa part
  if (usartx->CR1 & USART_CR1_OVER8)
	usart_divider = ((25*pclk)/(2*desired_baudrate)); // oversampling by 8 (i.e. sample each bit 8 times)
  else
	usart_divider = ((25*pclk)/(4*desired_baudrate)); // oversampling by 16

  // usart_divider is 1736.111 (as calculated above in comments)
  //
  // Mantissa (or lets say `i`)  X = 1736    i = X/100 => 17.36 (.36 will be discarded in unsigned data type)
  // Hence mantissa `i` will 17
  mantissa = usart_divider/100;
  temp_reg = mantissa << USART_BRR_DIV_Mantissa_Pos; // Bit 15:4, DIV_Mantissa[11:0], page: 838

  // Fraction (`F`) `F = X - (i x 100)`  =>  1736 - (17 x 100) = 36
  fraction = (usart_divider - (mantissa*100));

  if (usartx->CR1 & USART_CR1_OVER8)
	// RM page: 838
	// When OVER8=1, the DIV_Fraction3 bit (i.e. Bit-3) is not considered and must be kept cleared.
	fraction = (((fraction*8) + 50) /100) & ((uint8_t)0x07);
  else
	fraction = (((fraction*16) + 50) / 100) & ((uint8_t)0x0F);

  temp_reg |= fraction;                             // fraction part Bit 0:3, DIV_Fraction[3:0]

  usartx->BRR = temp_reg;
}
