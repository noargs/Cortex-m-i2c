#ifndef INC_STM32F446XX_RCC_DRIVER_H_
#define INC_STM32F446XX_RCC_DRIVER_H_

#include "stm32f4xx.h"

#define SYSTEM_CLOCK_SWS_HSI              0U
#define SYSTEM_CLOCK_SWS_HSE              1U
#define SYSTEM_CLOCK_SWS_PLL              2U
#define SYSTEM_CLOCK_SWS_PLL_R            3U

uint32_t RCC_GetPCLK1Value(void);
uint32_t RCC_GetPCLK2Value(void);
uint32_t RCC_GetPLLCLKValue(void);

#endif /* INC_STM32F446XX_RCC_DRIVER_H_ */
