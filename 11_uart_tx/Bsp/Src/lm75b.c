#include <stdio.h>
#include <string.h>
#include "lm75b.h"



static void lm75b_write8(uint8_t reg_address, uint8_t value);
static uint8_t lm75b_read8(uint8_t reg_address);
//static void lm75b_write16(uint8_t reg_address, uint16_t value);
static uint16_t lm75b_read16(uint8_t reg_address);
static void lm75b_gpio_i2c_pin_config();
static void lm75b_i2c_config();
static void lm75b_i2c_clear_busy_flag_erratum();

gpio_handle_t lm75b_i2c_scl, lm75b_i2c_sda;
i2c_handle_t  lm75b_i2c_handle;

void lm75b_init(void)
{
  lm75b_gpio_i2c_pin_config();
  lm75b_i2c_clear_busy_flag_erratum();
  lm75b_i2c_config();
  lm75b_i2c_handle.i2cx->CR1 |= I2C_CR1_PE;
  lm75b_i2c_handle.i2cx->CR1 |= I2C_CR1_ACK;

//  printf("Debug: ");
//  printf("[I2C Read]> %d\n", lm75b_read16(LM75B_ADDR_CONF));
}

static void lm75b_write8(uint8_t reg_address, uint8_t value)
{
  uint8_t tx[2];
  tx[0] = reg_address;
  tx[1] = value;
  I2C_MasterSendData(&lm75b_i2c_handle, tx, 2, LM75B_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
}

static uint8_t lm75b_read8(uint8_t reg_address)
{
  uint8_t data;
  I2C_MasterSendData(&lm75b_i2c_handle, &reg_address, 1, LM75B_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
  I2C_MasterReceiveData(&lm75b_i2c_handle, &data, 1, LM75B_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
  return data;
}

static uint16_t lm75b_read16(uint8_t reg_address)
{
  uint8_t data[2];
  I2C_MasterSendData(&lm75b_i2c_handle, &reg_address, 1, LM75B_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
  I2C_MasterReceiveData(&lm75b_i2c_handle, data, 2, LM75B_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
  return (uint16_t)(((uint16_t)data[0] << 8 ) | data[1]);
}

void lm75b_basic_temp(float *temp)
{
  uint16_t raw;

  raw = lm75b_read16(LM75B_ADDR_TEMP);
  raw = (raw >> 5);
  if ((raw & 0x0400) != 0)
  {
	raw = (raw | 0xF800U);
	*temp = (float)(-(~(raw) + 1)) * 0.125f;
  }
  else
  {
	*temp = (float)raw * 0.125f;
  }
}

static void lm75b_gpio_i2c_pin_config()
{
  memset(&lm75b_i2c_scl, 0, sizeof(lm75b_i2c_scl));
  memset(&lm75b_i2c_sda, 0, sizeof(lm75b_i2c_sda));

  lm75b_i2c_scl.gpiox                    = LM75B_I2C_GPIO_PORT;
  lm75b_i2c_scl.gpio_config.pin_alt_fun  = AF4;
  lm75b_i2c_scl.gpio_config.pin_mode     = LM75B_I2C_GPIO_PIN_MODE;
  lm75b_i2c_scl.gpio_config.pin_number   = LM75B_I2C_SCL_GPIO_PIN;
  lm75b_i2c_scl.gpio_config.pin_op_type  = LM75B_I2C_GPIO_OPTYPE;
  lm75b_i2c_scl.gpio_config.pin_pu_pd    = LM75B_I2C_GPIO_PUPD;
  lm75b_i2c_scl.gpio_config.pin_speed    = LM75B_I2C_GPIO_PIN_SPEED;
  GPIO_Init(&lm75b_i2c_scl);

  lm75b_i2c_sda.gpiox                    = LM75B_I2C_GPIO_PORT;
  lm75b_i2c_sda.gpio_config.pin_alt_fun  = AF4;
  lm75b_i2c_sda.gpio_config.pin_mode     = LM75B_I2C_GPIO_PIN_MODE;
  lm75b_i2c_sda.gpio_config.pin_number   = LM75B_I2C_SDA_GPIO_PIN;
  lm75b_i2c_sda.gpio_config.pin_op_type  = LM75B_I2C_GPIO_OPTYPE;
  lm75b_i2c_sda.gpio_config.pin_pu_pd    = LM75B_I2C_GPIO_PUPD;
  lm75b_i2c_sda.gpio_config.pin_speed    = LM75B_I2C_GPIO_PIN_SPEED;
  GPIO_Init(&lm75b_i2c_sda);
}

static void lm75b_i2c_config()
{
  lm75b_i2c_handle.i2cx                         = LM75B_I2C;
  lm75b_i2c_handle.i2c_config.i2c_ack_control   = LM75B_I2C_ACK_CONTROL;
  lm75b_i2c_handle.i2c_config.i2c_scl_speed     = LM75B_I2C_SPEED;
  I2C_Init(&lm75b_i2c_handle);
}

static void lm75b_i2c_clear_busy_flag_erratum()
{
  I2C_ClearBusyFlagErratum(&lm75b_i2c_scl, &lm75b_i2c_sda, &lm75b_i2c_handle);
}
