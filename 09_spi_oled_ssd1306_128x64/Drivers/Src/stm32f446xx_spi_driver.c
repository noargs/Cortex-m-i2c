#include "stm32f446xx_spi_driver.h"

void SPI_Init(spi_handle_t* spi_handle)
{
  SPI_PCLK_EN(spi_handle->spix);

  // Device mode: master or slave
  spi_handle->spix->CR1 |= spi_handle->spi_config.spi_device_mode << SPI_CR1_MSTR_Pos;    // MSTR bit RM page:886

  // Bus configuration: full-duplex, half-duplex, simplex-rx, simplex-tx
  if ((spi_handle->spi_config.spi_bus_config == SPI_BUS_CONFIG_FD) ||
	  (spi_handle->spi_config.spi_bus_config == SPI_BUS_CONFIG_SIMPLEX_TX)) // full-duplex but don't configure RX MISO so to make `Simplex TX only`
  {
	// BIDIMODE 15U should be cleared, 0 corresponde to 2-line unidirection data mode (i.e. full-duplex)
	spi_handle->spix->CR1 &= ~SPI_CR1_BIDIMODE;
	spi_handle->spix->CR1 &= ~SPI_CR1_RXONLY;                               // 0: full-duplex, 1: Rx only (output disabled)
  }
  else if (spi_handle->spi_config.spi_bus_config == SPI_BUS_CONFIG_HD)
  {
	spi_handle->spix->CR1 |= SPI_CR1_BIDIMODE;
  }
  else if (spi_handle->spi_config.spi_bus_config == SPI_BUS_CONFIG_SIMPLEX_RX)
  {
	spi_handle->spix->CR1 &= ~SPI_CR1_BIDIMODE;
	spi_handle->spix->CR1 |= SPI_CR1_RXONLY;
  }

  // Baudrate
  spi_handle->spix->CR1 |= spi_handle->spi_config.spi_sck_speed << SPI_CR1_BR_Pos;

  // DFF: data frame format
  spi_handle->spix->CR1 |= spi_handle->spi_config.spi_dff << SPI_CR1_DFF_Pos;

  // CPOL: clock polarity
  spi_handle->spix->CR1 |= spi_handle->spi_config.spi_cpol << SPI_CR1_CPOL_Pos;

  // CPHA: clock phase
  spi_handle->spix->CR1 |= spi_handle->spi_config.spi_cpha << SPI_CR1_CPHA_Pos;

  // SSM: software slave management
  spi_handle->spix->CR1 |= spi_handle->spi_config.spi_ssm << SPI_CR1_SSM_Pos;

  /*
   * making SSOE 1 does NSS output enable.
   * The NSS pin is automatically managed by the hardware.
   * i.e. when SPE=1, NSS will be pulled to low
   * and NSS pin will be high when SPE=0
   */
  spi_handle->spix->CR2 |= SPI_CR2_SSOE;

  // ENABLE SPI PERIPHERAL
  //spi_handle->spix->CR1 |= SPI_CR1_SPE;
  SPI_PeripheralControl(spi_handle->spix, ENABLE);
}

void SPI_SendData(SPI_TypeDef* spix, uint8_t* tx_buffer, uint32_t length)
{
  while (length > 0)
  {
	// wait till txe is empty
	while ((spix->SR & SPI_SR_TXE) == RESET);

	// check the DFF bit in CR1 for 8 or 16 bit transfer
	if (spix->CR1 & SPI_CR1_DFF)
	{
	  // 16bit
	  spix->DR = *((uint16_t*)tx_buffer);
	  length--;
	  length--;
	  (uint16_t*)tx_buffer++;
	}
	else
	{
	  spix->DR = *tx_buffer;
	  length--;
	  tx_buffer++;
	}
  }

//  // CONFIRM THE SPI IS NOT BUSY
//  while ((spix->SR & SPI_SR_BSY) == SET);
//
//  // DISABLE SPI PERIPHERAL AFTER DATA COMMUNICATION
//  spix->CR1 &= ~SPI_CR1_SPE;
}

void SPI_PeripheralControl(SPI_TypeDef* spix, uint8_t ENABLE_OR_DISABLE)
{
  if (ENABLE_OR_DISABLE == ENABLE) {
	  spix->CR1 |= SPI_CR1_SPE;
  } else {
	  spix->CR1 &= ~SPI_CR1_SPE;
  }
}
