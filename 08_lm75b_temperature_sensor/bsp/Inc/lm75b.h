#ifndef INC_LM75B_H_
#define INC_LM75B_H_

#include <stdint.h>
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_i2c_driver.h"

#define LM75B_I2C_GPIO_PORT           GPIOB
#define LM75B_I2C_SCL_GPIO_PIN        GPIO_PIN_8
#define LM75B_I2C_SDA_GPIO_PIN        GPIO_PIN_9
#define LM75B_I2C_GPIO_PIN_MODE       GPIO_MODE_ALTFN
#define LM75B_I2C_GPIO_PIN_SPEED      GPIO_SPEED_FAST
#define LM75B_I2C_GPIO_OPTYPE         GPIO_OP_TYPE_OD
#define LM75B_I2C_GPIO_PUPD           GPIO_PUPD_NO
#define AF4                           4


#define LM75B_I2C                     I2C1
#define LM75B_I2C_ACK_CONTROL         I2C_ACK_ENABLE
#define LM75B_I2C_SPEED               I2C_SCL_SPEED_SM
#define LM75B_I2C_DEVICE_ADDR         0x48

#define LM75B_ADDR_TEMP               0x00
#define LM75B_ADDR_CONF               0x01
#define LM75B_ADDR_THYST              0x02
#define LM75B_ADDR_TOS                0x03

#define LM75B_TEMPERATURE_NEGATIVE    0x1
#define LM75B_TEMPERATURE_POSITIVE    0x0

void lm75b_init(void);
void lm75b_basic_temperature(float* temp);


#endif /* INC_LM75B_H_ */
