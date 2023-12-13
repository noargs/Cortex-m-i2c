#ifndef INC_STM32F446XX_USART_DRIVER_H_
#define INC_STM32F446XX_USART_DRIVER_H_

#include "stm32f4xx.h"

typedef struct
{
  uint8_t  usart_mode;
  uint32_t usart_baudrate;
  uint8_t  usart_no_of_stop_bits;
  uint8_t  usart_word_length;
  uint8_t  usart_parity_control;
  uint8_t  usart_hw_flow_control;
}usart_config_t;

typedef struct
{
  USART_TypeDef *usartx;
  usart_config_t usart_config;
}usart_handle_t;

// @usart_mode
#define USART_MODE_TX_ONLY                0U
#define USART_MODE_RX_ONLY                1U
#define USART_MODE_TX_AND_RX              2U

// @usart_baudrate
#define USART_STD_BAUD_1200               1200
#define USART_STD_BAUD_2400               2400
#define USART_STD_BAUD_9600               9600
#define USART_STD_BAUD_19200              19200
#define USART_STD_BAUD_38400              34800
#define USART_STD_BAUD_57600              57600
#define USART_STD_BAUD_115200             115200
#define USART_STD_BAUD_230400             230400
#define USART_STD_BAUD_460800             460800
#define USART_STD_BAUD_921600             921600
#define USART_STD_BAUD_2M                 2000000
#define USART_STD_BAUD_3M                 3000000

// @usart_no_of_stop_bits
#define USART_STOP_BITS_1                 0
#define USART_STOP_BITS_0_5               1
#define USART_STOP_BITS_2                 2
#define USART_STOP_BITS_1_5               3

// @usart_word_length
#define USART_WORDLEN_8BITS               0
#define USART_WORDLEN_9BITS               1

// @usart_parity_control
#define USART_PARITY_DISABLE              0
#define USART_PARITY_EN_EVEN              1
#define USART_PARITY_EN_ODD               2

// @usart_hw_flow_control
#define USART_HW_FLOW_CONTROL_NONE        0
#define USART_HW_FLOW_CONTROL_CTS         1
#define USART_HW_FLOW_CONTROL_RTS         2
#define USART_HW_FLOW_CONTROL_CTS_RTS     3


#define USART_PCLK_EN(x)                  ((x == USART1) ? (RCC->APB2ENR |= RCC_APB2ENR_USART1EN) :\
	                                       (x == USART2) ? (RCC->APB1ENR |= RCC_APB1ENR_USART2EN) :\
                                           (x == USART3) ? (RCC->APB1ENR |= RCC_APB1ENR_USART3EN) : 0)


void USART_Init(usart_handle_t *usart_handle);
void USART_PeripheralControl(USART_TypeDef* usartx, uint8_t ENABLE_OR_DISABLE);

void USART_SendData(usart_handle_t* usart_handle, uint8_t* tx_buffer, uint32_t length);
void USART_ReceiveData(usart_handle_t* usart_handle, uint8_t* rx_buffer, uint32_t length);
void USART_SendChar(USART_TypeDef *usartx, int ch);



#endif /* INC_STM32F446XX_USART_DRIVER_H_ */
