#ifndef INC_STM32F446XX_USART_DRIVER_H_
#define INC_STM32F446XX_USART_DRIVER_H_

#include "stm32f4xx.h"

typedef struct
{
  uint16_t usart_baud_rate;
}usart_config_t;

typedef struct
{
  USART_TypeDef *usartx;
  usart_config_t usart_config;
}usart_handle_t;

#define USART_DESIRED_BAUD_RATE           115200
#define USART_SYSTEM_CLOCK_FREQ           16000000


#define USART_PCLK_EN(x)                  ((x == USART1) ? (RCC->APB2ENR |= RCC_APB2ENR_USART1EN) :\
	                                       (x == USART2) ? (RCC->APB1ENR |= RCC_APB1ENR_USART2EN) :\
                                           (x == USART3) ? (RCC->APB1ENR |= RCC_APB1ENR_USART3EN) : 0)


void USART_Init(usart_handle_t *usart_handle);
void USART_SendData(USART_TypeDef *usartx, int ch);



#endif /* INC_STM32F446XX_USART_DRIVER_H_ */
