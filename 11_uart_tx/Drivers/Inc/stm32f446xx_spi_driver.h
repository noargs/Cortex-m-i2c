#ifndef INC_STM32F446XX_SPI_DRIVER_H_
#define INC_STM32F446XX_SPI_DRIVER_H_

#include "stm32f4xx.h"

typedef struct
{
  uint8_t spi_device_mode;
  uint8_t spi_bus_config;
  uint8_t spi_sck_speed;
  uint8_t spi_dff;
  uint8_t spi_cpol;
  uint8_t spi_cpha;
  uint8_t spi_ssm;
} spi_config_t;

typedef struct
{
  SPI_TypeDef* spix;
  spi_config_t spi_config;
} spi_handle_t;

// @spi_device_mode
#define SPI_DEVICE_MODE_SLAVE             0
#define SPI_DEVICE_MODE_MASTER            1

// @spi_bus_config
#define SPI_BUS_CONFIG_FD                 1
#define SPI_BUS_CONFIG_HD                 2
#define SPI_BUS_CONFIG_SIMPLEX_RX         3
#define SPI_BUS_CONFIG_SIMPLEX_TX         4

// @spi_sclk_speed
#define SPI_SCLK_SPEED_DIV2               0
#define SPI_SCLK_SPEED_DIV4               1
#define SPI_SCLK_SPEED_DIV8               2
#define SPI_SCLK_SPEED_DIV16              3
#define SPI_SCLK_SPEED_DIV32              4
#define SPI_SCLK_SPEED_DIV64              5
#define SPI_SCLK_SPEED_DIV128             6
#define SPI_SCLK_SPEED_DIV256             7

// @spi_dff
#define SPI_DFF_8BITS                     0
#define SPI_DFF_16BITS                    1

// @spi_cpol
#define SPI_CPOL_LOW                      0
#define SPI_CPOL_HIGH                     1

// @spi_cpha
#define SPI_CPHA_LOW                      0
#define SPI_CPHA_HIGH                     1

// @spi_ssm
#define SPI_SSM_DISABLE                   0 // Default Hardware slave management (i.e. Software slave management disabled)
#define SPI_SSM_ENABLE                    1

#define SPI_PCLK_EN(x)                    ((x == SPI1) ? (RCC->APB2ENR |= RCC_APB2ENR_SPI1EN) :\
                                           (x == SPI2) ? (RCC->APB1ENR |= RCC_APB1ENR_SPI2EN) :\
                                           (x == SPI3) ? (RCC->APB1ENR |= RCC_APB1ENR_SPI3EN) :\
                                           (x == SPI4) ? (RCC->APB2ENR |= RCC_APB2ENR_SPI4EN) :0)

#define SPI_PCLK_DIS(x)                   ((x == SPI1) ? (RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN) :\
                                           (x == SPI2) ? (RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN) :\
                                           (x == SPI3) ? (RCC->APB1ENR &= ~RCC_APB1ENR_SPI3EN) :\
                                           (x == SPI4) ? (RCC->APB2ENR &= ~RCC_APB2ENR_SPI4EN) :0)

void SPI_Init(spi_handle_t* spi_handle);
void SPI_SendData(SPI_TypeDef* spix, uint8_t* tx_buffer, uint32_t length);
void SPI_PeripheralControl(SPI_TypeDef* spix, uint8_t ENABLE_OR_DISABLE);



#endif /* INC_STM32F446XX_SPI_DRIVER_H_ */
