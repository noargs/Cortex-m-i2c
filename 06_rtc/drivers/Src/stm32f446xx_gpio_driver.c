#include "stm32f446xx_gpio_driver.h"

void GPIO_Init(gpio_handle_t *gpio_h)
{
  uint32_t temp = 0;

  // Enabe the clock access
  GPIO_PCLK_EN(gpio_h->gpiox);

  if (gpio_h->gpio_config.pin_mode <= GPIO_MODE_ANALOG)
  {
	// Non-interrupt mode
	gpio_h->gpiox->MODER &= ~(0x3 << (2 * gpio_h->gpio_config.pin_number));

	temp = (gpio_h->gpio_config.pin_mode << (2 * gpio_h->gpio_config.pin_number));
	gpio_h->gpiox->MODER |= temp;
  }
  else
  {
	// Interrupt mode
  }

  // OTYPER
  temp = 0;
  gpio_h->gpiox->OTYPER &= ~(0x1 << gpio_h->gpio_config.pin_number);

  temp = gpio_h->gpio_config.pin_op_type << gpio_h->gpio_config.pin_number;
  gpio_h->gpiox->OTYPER |= temp;

  // OSPEEDR
  temp = 0;
  gpio_h->gpiox->OSPEEDR &= ~(0x3 << (2 * gpio_h->gpio_config.pin_number));

  temp = (gpio_h->gpio_config.pin_speed << (2 * gpio_h->gpio_config.pin_number));
  gpio_h->gpiox->OSPEEDR |= temp;

  // PUPDR
  temp = 0;
  gpio_h->gpiox->PUPDR &= ~(0x3 << (2 * gpio_h->gpio_config.pin_number));

  temp = (gpio_h->gpio_config.pin_pu_pd << (2 * gpio_h->gpio_config.pin_number));
  gpio_h->gpiox->PUPDR |= temp;

  // AFRL, AFRH
  if (gpio_h->gpio_config.pin_mode == GPIO_MODE_ALTFN)
  {
	GPIO_ConfigureAlternateFunction(gpio_h);
  }

}

uint8_t GPIO_ReadFromInputPin(GPIO_TypeDef *gpiox, uint8_t pin_number)
{
  return ((gpiox->IDR >> pin_number) & 0x00000001);
}

void GPIO_ConfigureAlternateFunction(gpio_handle_t *gpio_handle)
{
  uint8_t temp1, temp2;
  temp1 = gpio_handle->gpio_config.pin_number / 8;
  temp2 = gpio_handle->gpio_config.pin_number % 8;
  gpio_handle->gpiox->AFR[temp1] &= ~(0xF << (4 * temp2));
  gpio_handle->gpiox->AFR[temp1] |= (gpio_handle->gpio_config.pin_alt_fun << (4 * temp2));
}
