#include <stdio.h>
#include <string.h>
#include "lm75b.h"

static void lm75b_gpio_i2c_pin_config(void);
static void lm75b_i2c_config(void);
static void lm75b_i2c_clear_busy_flag_erratum(void);


gpio_handle_t lm75b_scl, lm75b_sda;
i2c_handle_t  lm75b_i2c_handle;


void lm75b_init(void)
{
  lm75b_gpio_i2c_pin_config();
  lm75b_i2c_clear_busy_flag_erratum();
  lm75b_i2c_config();
}

static void lm75b_gpio_i2c_pin_config(void)
{
  memset(&lm75b_scl,0,sizeof(gpio_handle_t));
  memset(&lm75b_sda,0,sizeof(gpio_handle_t));

  lm75b_scl.gpiox                     = LM75B_I2C_GPIO_PORT;
  lm75b_scl.gpio_config.pin_alt_fun   = AF4;
  lm75b_scl.gpio_config.pin_mode      = LM75B_I2C_GPIO_PIN_MODE;
  lm75b_scl.gpio_config.pin_number    = LM75B_I2C_SCL_GPIO_PIN;
  lm75b_scl.gpio_config.pin_op_type   = LM75B_I2C_GPIO_OPTYPE;
  lm75b_scl.gpio_config.pin_pu_pd     = LM75B_I2C_GPIO_PUPD;
  lm75b_scl.gpio_config.pin_speed     = LM75B_I2C_GPIO_PIN_SPEED;
  GPIO_Init(&lm75b_scl);

  lm75b_sda.gpiox                     = LM75B_I2C_GPIO_PORT;
  lm75b_sda.gpio_config.pin_alt_fun   = AF4;
  lm75b_sda.gpio_config.pin_mode      = LM75B_I2C_GPIO_PIN_MODE;
  lm75b_sda.gpio_config.pin_number    = LM75B_I2C_SDA_GPIO_PIN;
  lm75b_sda.gpio_config.pin_op_type   = LM75B_I2C_GPIO_OPTYPE;
  lm75b_sda.gpio_config.pin_pu_pd     = LM75B_I2C_GPIO_PUPD;
  lm75b_sda.gpio_config.pin_speed     = LM75B_I2C_GPIO_PIN_SPEED;
  GPIO_Init(&lm75b_sda);
}

static void lm75b_i2c_config(void)
{
  lm75b_i2c_handle.i2cx                        = LM75B_I2C;
  lm75b_i2c_handle.i2c_config.i2c_ack_control  = LM75B_I2C_ACK_CONTROL;
  lm75b_i2c_handle.i2c_config.i2c_scl_speed    = LM75B_I2C_SPEED;
  I2C_Init(&lm75b_i2c_handle);
}

static void lm75b_i2c_clear_busy_flag_erratum(void)
{
  I2C_ClearBusyFlagErratum(&lm75b_scl, &lm75b_sda, &lm75b_i2c_handle);
}
