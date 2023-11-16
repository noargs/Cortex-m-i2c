#ifndef INC_STM32F446XX_GPIO_DRIVER_H_
#define INC_STM32F446XX_GPIO_DRIVER_H_

#include "stm32f4xx.h"

typedef struct
{
  uint8_t pin_number;                 // @pin_number
  uint8_t pin_mode;                   // @pin_mode
  uint8_t pin_speed;                  // @pin_speed
  uint8_t pin_pu_pd        ;          // @pin_pu_pd
  uint8_t pin_op_type;                // @pin_op_type
  uint8_t pin_alt_fun;                // @pin_alt_fun
}gpio_config_t;

typedef struct
{
  GPIO_TypeDef   *gpiox;
  gpio_config_t  gpio_config;
}gpio_handle_t;

// @pin_op_type
#define GPIO_OP_TYPE_PP               0       // output type push-pull
#define GPIO_OP_TYPE_OD               1       // output type open-drain

// @pin_mode
#define GPIO_MODE_IN                  0
#define GPIO_MODE_OUT                 1
#define GPIO_MODE_ALTFN               2
#define GPIO_MODE_ANALOG              3
#define GPIO_MODE_IT_FT               4       // input falling-edge trigger
#define GPIO_MODE_IT_RT               5       // input rising-edge trigger
#define GPIO_MODE_IT_RFT              6       // input rising-falling-edge trigger

// @pin_pu_pd
#define GPIO_PUPD_NO                  0       // no pull-up, pull-down
#define GPIO_PUPD_PU                  1       // pull-up
#define GPIO_PUPD_PD                  2       // pull-down

// @pin_speed
#define GPIO_SPEED_LOW                0
#define GPIO_SPEED_MEDIUM             1
#define GPIO_SPEED_FAST               2
#define GPIO_SPEED_HIGH               3

// @pin_number
#define GPIO_PIN_0                    0
#define GPIO_PIN_1                    1
#define GPIO_PIN_2                    2
#define GPIO_PIN_3                    3
#define GPIO_PIN_4                    4
#define GPIO_PIN_5                    5
#define GPIO_PIN_6                    6
#define GPIO_PIN_7                    7
#define GPIO_PIN_8                    8
#define GPIO_PIN_9                    9
#define GPIO_PIN_10                   10
#define GPIO_PIN_11                   11
#define GPIO_PIN_12                   12
#define GPIO_PIN_13                   13
#define GPIO_PIN_14                   14
#define GPIO_PIN_15                   15

#define GPIO_PIN_SET                  ENABLE
#define GPIO_PIN_RESET                DISABLE


#define GPIO_PCLK_EN(x)               ((x == GPIOA) ? (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN) :\
                                       (x == GPIOB) ? (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN) :\
                                       (x == GPIOC) ? (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN) :\
                                       (x == GPIOD) ? (RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN) :\
                                       (x == GPIOE) ? (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN) :\
                                       (x == GPIOF) ? (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN) :\
                                       (x == GPIOG) ? (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN) :\
                                       (x == GPIOH) ? (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN) : 0)

#define GPIO_PCLK_DIS(x)              ((x == GPIOA) ? (RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN) :\
                                       (x == GPIOB) ? (RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN) :\
                                       (x == GPIOC) ? (RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN) :\
                                       (x == GPIOD) ? (RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIODEN) :\
                                       (x == GPIOE) ? (RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOEEN) :\
                                       (x == GPIOF) ? (RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOFEN) :\
                                       (x == GPIOG) ? (RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOGEN) :\
                                       (x == GPIOH) ? (RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOHEN) : 0)

#define GPIO_PIN_SET                  ENABLE
#define GPIO_PIN_RESET                DISABLE

void GPIO_Init(gpio_handle_t *gpio_handle);
uint8_t GPIO_ReadFromInputPin(GPIO_TypeDef *gpiox, uint8_t pin_number);
void GPIO_WriteToOutputPin(GPIO_TypeDef *gpiox, uint8_t pin_number, uint8_t data_to_write);



#endif /* INC_STM32F446XX_GPIO_DRIVER_H_ */
