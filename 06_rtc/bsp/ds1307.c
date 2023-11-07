#include <string.h>
#include "ds1307.h"

static void ds1307_gpio_i2c_pin_config(void);
static void ds1307_i2c_config(void);


uint8_t ds1307_init(void)
{
  ds1307_gpio_i2c_pin_config();
  ds1307_i2c_config();
}

static void ds1307_gpio_i2c_pin_config()
{
  gpio_handle_t i2c_sda, i2c_scl;

  memset(&i2c_sda, 0, sizeof(i2c_sda));
  memset(&i2c_scl, 0, sizeof(i2c_scl));

  i2c_sda.gpiox                    = DS1307_I2C_GPIO_PORT;
  i2c_sda.gpio_config.pin_alt_fun  = AF4;
  i2c_sda.gpio_config.pin_mode     = DS1307_I2C_GPIO_PIN_MODE;
  i2c_sda.gpio_config.pin_number   = DS1307_I2C_SDA_GPIO_PIN;
  i2c_sda.gpio_config.pin_op_type  = DS1307_I2C_GPIO_OPTYPE;
  i2c_sda.gpio_config.pin_pu_pd    = DS1307_I2C_GPIO_PUPD;
  i2c_sda.gpio_config.pin_speed    = DS1307_I2C_GPIO_PIN_SPEED;
  GPIO_Init(&i2c_sda);

  i2c_scl.gpiox                    = DS1307_I2C_GPIO_PORT;
  i2c_scl.gpio_config.pin_alt_fun  = AF4;
  i2c_scl.gpio_config.pin_mode     = DS1307_I2C_GPIO_PIN_MODE;
  i2c_scl.gpio_config.pin_number   = DS1307_I2C_SCL_GPIO_PIN;
  i2c_scl.gpio_config.pin_op_type  = DS1307_I2C_GPIO_OPTYPE;
  i2c_scl.gpio_config.pin_pu_pd    = DS1307_I2C_GPIO_PUPD;
  i2c_scl.gpio_config.pin_speed    = DS1307_I2C_GPIO_PIN_SPEED;
  GPIO_Init(&i2c_scl);
}

static void ds1307_i2c_config()
{
  i2c_handle_t ds1307_i2c_h;

  ds1307_i2c_h.i2cx                          = DS1307_I2C;
  ds1307_i2c_h.i2c_config.i2c_ack_control    = DS1307_I2C_ACK_CONTROL;
  ds1307_i2c_h.i2c_config.i2c_scl_speed      = DS1307_I2C_SPEED;
  ds1307_i2c_h.i2c_config.i2c_device_address = DS1307_I2C_DEVICE_ADDR;
  I2C_Init(&ds1307_i2c_h);
}
