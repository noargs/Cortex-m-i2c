#include <string.h>
#include "ds1307.h"

static void ds1307_gpio_i2c_pin_config(void);
static void ds1307_i2c_config(void);
static void ds1307_write(uint8_t reg_address, uint8_t value);
static uint8_t ds1307_read(uint8_t reg_address);
static void ds1307_i2c_clear_busy_flag_erratum();

gpio_handle_t i2c_scl, i2c_sda;
i2c_handle_t  ds1307_i2c_h;

uint8_t ds1307_init(void)
{
  ds1307_gpio_i2c_pin_config();

  ds1307_i2c_clear_busy_flag_erratum();

  ds1307_i2c_config();

  ds1307_i2c_h.i2cx->CR1 |= I2C_CR1_PE;

  ds1307_i2c_h.i2cx->CR1 |= I2C_CR1_ACK;

  // ds1307_read(DS1307_ADDR_SEC); // return 0x05  =>  (bit0)0000 010(bit7,CH)1 => [CH=1 clock halted]

  ds1307_write(DS1307_ADDR_SEC, 0x00);

  uint8_t clock_state = ds1307_read(DS1307_ADDR_SEC);

  // return `1` (i.e. CH=1), `Initialisation Failed`
  // return `0` (i.e. CH=0), `Initialisation Successful`
  return ((clock_state >> 7) & 0x1);
}

static void ds1307_write(uint8_t reg_address, uint8_t value)
{
  uint8_t tx[2];
  tx[0] = reg_address;
  tx[1] = value;

  I2C_MasterSendData(&ds1307_i2c_h, tx, 2, DS1307_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
}

static uint8_t ds1307_read(uint8_t reg_address)
{
  uint8_t data;
  I2C_MasterSendData(&ds1307_i2c_h, &reg_address, 1, DS1307_I2C_DEVICE_ADDR, I2C_SR_ENABLE);
  I2C_MasterReceiveData(&ds1307_i2c_h, &data, 1, DS1307_I2C_DEVICE_ADDR, I2C_SR_DISABLE);
  return data;
}

static void ds1307_gpio_i2c_pin_config()
{
  memset(&i2c_scl, 0, sizeof(i2c_scl));
  memset(&i2c_sda, 0, sizeof(i2c_sda));


  i2c_scl.gpiox                    = DS1307_I2C_GPIO_PORT;
  i2c_scl.gpio_config.pin_alt_fun  = AF4;
  i2c_scl.gpio_config.pin_mode     = DS1307_I2C_GPIO_PIN_MODE;
  i2c_scl.gpio_config.pin_number   = DS1307_I2C_SCL_GPIO_PIN;
  i2c_scl.gpio_config.pin_op_type  = DS1307_I2C_GPIO_OPTYPE;
  i2c_scl.gpio_config.pin_pu_pd    = DS1307_I2C_GPIO_PUPD;
  i2c_scl.gpio_config.pin_speed    = DS1307_I2C_GPIO_PIN_SPEED;
  GPIO_Init(&i2c_scl);

  i2c_sda.gpiox                    = DS1307_I2C_GPIO_PORT;
  i2c_sda.gpio_config.pin_alt_fun  = AF4;
  i2c_sda.gpio_config.pin_mode     = DS1307_I2C_GPIO_PIN_MODE;
  i2c_sda.gpio_config.pin_number   = DS1307_I2C_SDA_GPIO_PIN;
  i2c_sda.gpio_config.pin_op_type  = DS1307_I2C_GPIO_OPTYPE;
  i2c_sda.gpio_config.pin_pu_pd    = DS1307_I2C_GPIO_PUPD;
  i2c_sda.gpio_config.pin_speed    = DS1307_I2C_GPIO_PIN_SPEED;
  GPIO_Init(&i2c_sda);
}

static void ds1307_i2c_config()
{
  ds1307_i2c_h.i2cx                          = DS1307_I2C;
  ds1307_i2c_h.i2c_config.i2c_ack_control    = DS1307_I2C_ACK_CONTROL;
  ds1307_i2c_h.i2c_config.i2c_scl_speed      = DS1307_I2C_SPEED;
  I2C_Init(&ds1307_i2c_h);
}

static void ds1307_i2c_clear_busy_flag_erratum()
{
  // https://www.st.com/content/ccc/resource/technical/document/errata_sheet/7f/05/b0/bc/34/2f/4c/21/CD00288116.pdf/files/CD00288116.pdf/jcr:content/translations/en.CD00288116.pdf
  // [On Page: 19]
  //
  //1. Disable the I2C peripheral
  ds1307_i2c_h.i2cx->CR1 &= ~I2C_CR1_PE;

  //2. scl and sda i/o as general purpose output open-drain and high level (write 1 to GPIOx_ODR)
  i2c_scl.gpiox->MODER &= ~(0x3 << (2 * i2c_scl.gpio_config.pin_number));
  i2c_scl.gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_scl.gpio_config.pin_number));
  i2c_scl.gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_scl.gpio_config.pin_number);
  i2c_scl.gpiox->ODR |= (0x1UL << i2c_scl.gpio_config.pin_number);

  i2c_sda.gpiox->MODER &= ~(0x3 << (2 * i2c_sda.gpio_config.pin_number));
  i2c_sda.gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_sda.gpio_config.pin_number));
  i2c_sda.gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_sda.gpio_config.pin_number);
  i2c_sda.gpiox->ODR |= (0x1UL << i2c_sda.gpio_config.pin_number);

  //3. check scl and sda High level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_scl.gpiox, i2c_scl.gpio_config.pin_number) == RESET);
  while (GPIO_ReadFromInputPin(i2c_sda.gpiox, i2c_sda.gpio_config.pin_number) == RESET);

  //4. configure the sda i/o as general purpose output open-drain and low level (write 0 to GPIOx_ODR)
  i2c_sda.gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_sda.gpio_config.pin_number));
  i2c_sda.gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_sda.gpio_config.pin_number);
  i2c_sda.gpiox->ODR &= ~(0x1UL << i2c_sda.gpio_config.pin_number);

  //5. check sda low level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_sda.gpiox, i2c_sda.gpio_config.pin_number) != RESET);

  //6. configure the scl i/o as general purpose output open-drain and low level (write 0 to GPIOx_ODR)
  i2c_scl.gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_scl.gpio_config.pin_number));
  i2c_scl.gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_scl.gpio_config.pin_number);
  i2c_scl.gpiox->ODR &= ~(0x1UL << i2c_scl.gpio_config.pin_number);

  //7. check scl low level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_scl.gpiox, i2c_scl.gpio_config.pin_number) != RESET);

  //8. configure the scl i/o as general purpose output open-drain and high level (write 1 to GPIOx_ODR)
  i2c_scl.gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_scl.gpio_config.pin_number));
  i2c_scl.gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_scl.gpio_config.pin_number);
  i2c_scl.gpiox->ODR |= (0x1UL << i2c_scl.gpio_config.pin_number);

  //9. check scl High level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_scl.gpiox, i2c_scl.gpio_config.pin_number) == RESET);

  //10. configure the sda i/o as general purpose output open-drain and high level (write 1 to GPIOx_ODR)
  i2c_sda.gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_sda.gpio_config.pin_number));
  i2c_sda.gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_sda.gpio_config.pin_number);
  i2c_sda.gpiox->ODR |= (0x1UL << i2c_sda.gpio_config.pin_number);

  //11. check sda High level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_sda.gpiox, i2c_sda.gpio_config.pin_number) == RESET);

  //12. configure the scl and sda i/o's as Alternate function open-drain
//  GPIO_ConfigureAlternateFunction(&i2c_scl);
  i2c_scl.gpiox->MODER &= ~(0x3 << (2 * i2c_scl.gpio_config.pin_number));
  i2c_scl.gpiox->MODER |= (GPIO_MODE_ALTFN << (2 * i2c_scl.gpio_config.pin_number));
  i2c_scl.gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_scl.gpio_config.pin_number);
//  GPIO_ConfigureAlternateFunction(&i2c_sda);
  i2c_sda.gpiox->MODER &= ~(0x3 << (2 * i2c_sda.gpio_config.pin_number));
  i2c_sda.gpiox->MODER |= (GPIO_MODE_ALTFN << (2 * i2c_sda.gpio_config.pin_number));
  i2c_sda.gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_sda.gpio_config.pin_number);

  //13. set SWRST bit in I2Cx_CR1 register.
  ds1307_i2c_h.i2cx->CR1 |= I2C_CR1_SWRST;

  //14. clear SWRST bit in I2Cx_CR1 register.
  ds1307_i2c_h.i2cx->CR1 &= ~I2C_CR1_SWRST;

  //15. enable the I2C peripheral by setting PE bit in the I2Cx_CR1 register
  ds1307_i2c_h.i2cx->CR1 |= I2C_CR1_PE;
}

















