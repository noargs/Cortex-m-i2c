#include <stdio.h>
#include <string.h>
#include "lm75b.h"

static uint16_t lm75b_read16(uint8_t reg_address);

static void lm75b_gpio_i2c_pin_config(void);
static void lm75b_i2c_config(void);
static void lm75b_i2c_enable(void);
static void lm75b_i2c_ack_enable(void);
static void lm75b_i2c_clear_busy_flag_erratum(void);


gpio_handle_t lm75b_scl, lm75b_sda;
i2c_handle_t  lm75b_i2c_handle;


void lm75b_init(void)
{
  lm75b_gpio_i2c_pin_config();
  lm75b_i2c_clear_busy_flag_erratum();
  lm75b_i2c_config();
  lm75b_i2c_enable();
  lm75b_i2c_ack_enable();
}

void lm75b_get_temperature(float* temp)
{
  uint16_t temp_data;

  temp_data = lm75b_read16(LM75B_ADDR_TEMP); // Temperature register address 0x00
  temp_data = (temp_data >> 5);              // [MSB + LSB] = 16-bits out of 11 bits implemented,
                                             // 0-4bits in LSB should be ignored [RM page: 7]

  if ((temp_data >> 10) & 0x1)               // or ((raw & 0x0400) == 1)     i.e. 10-bit=1
  {
	// MSB D10=1 means negative temperature `temp_value(°C) = -(2's complement of temp data) x 0.125°C`

	// [15th bit ...], [11th bit D10 D9 D8], [D7...D4], [D3...D0]
	//    F (1111)          8 (1000)           0          0
	temp_data = (temp_data | 0xF800U);                             // Set negative part - only changing 15th to 11th bit into 1s (i.e. 1111 1)
	*temp = (float)(-(~temp_data + 1)) * 0.125f;                   // flip temp_data and add 1 in order to make it 2's complement
  }
  else
  {
	// MSB D10=0 means positive temperature `temp_value(°C) = +(Temp data) x 0.125°C`
	*temp = (float)temp_data * 0.125f;
  }
}

static uint16_t lm75b_read16(uint8_t reg_address)
{
  uint8_t buf[2];
  I2C_MasterSendData(&lm75b_i2c_handle, &reg_address, 1, LM75B_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
  I2C_MasterReceiveData(&lm75b_i2c_handle, buf, 2, LM75B_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
  return (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
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

static void lm75b_i2c_enable(void)
{
  lm75b_i2c_handle.i2cx->CR1 |= I2C_CR1_PE;
}

static void lm75b_i2c_ack_enable(void)
{
  lm75b_i2c_handle.i2cx->CR1 |= I2C_CR1_ACK;
}

static void lm75b_i2c_clear_busy_flag_erratum(void)
{
  I2C_ClearBusyFlagErratum(&lm75b_scl, &lm75b_sda, &lm75b_i2c_handle);
}
