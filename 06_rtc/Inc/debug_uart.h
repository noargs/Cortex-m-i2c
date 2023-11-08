#ifndef DEBUG_UART_H_
#define DEBUG_UART_H_

#include "stm32f446xx_usart_driver.h"
#include "stm32f446xx_gpio_driver.h"

#define DEBUG_UART_GPIO_PORT          GPIOA
#define DEBUG_UART_TX_GPIO_PIN        GPIO_PIN_2
#define DEBUG_UART_RX_GPIO_PIN        GPIO_PIN_3
#define DEBUG_UART_GPIO_PIN_MODE      GPIO_MODE_ALTFN
#define DEBUG_UART_GPIO_PIN_SPEED     GPIO_SPEED_FAST
#define AF7                           7U

#define DEBUG_UART_APB1_FREQ          USART_SYSTEM_CLOCK_FREQ
#define DEBUG_UART_DESIRED_BAUD       USART_DESIRED_BAUD_RATE


void DebugUART_Init(void);




#endif /* DEBUG_UART_H_ */
