#ifndef DS1307_H_
#define DS1307_H_

#include <stdint.h>
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_i2c_driver.h"

#define DS1307_I2C_GPIO_PORT          GPIOB
#define DS1307_I2C_SCL_GPIO_PIN       GPIO_PIN_8
#define DS1307_I2C_SDA_GPIO_PIN       GPIO_PIN_9
#define DS1307_I2C_GPIO_PIN_MODE      GPIO_MODE_ALTFN
#define DS1307_I2C_GPIO_PIN_SPEED     GPIO_SPEED_FAST
#define DS1307_I2C_GPIO_OPTYPE        GPIO_OP_TYPE_OD
#define DS1307_I2C_GPIO_PUPD          GPIO_PUPD_NO
#define AF4                           4


#define DS1307_I2C                    I2C1
#define DS1307_I2C_ACK_CONTROL        I2C_ACK_ENABLE
#define DS1307_I2C_SPEED              I2C_SCL_SPEED_SM
#define DS1307_I2C_DEVICE_ADDR        0x68

uint8_t ds1307_init(void);



#endif /* DS1307_H_ */
