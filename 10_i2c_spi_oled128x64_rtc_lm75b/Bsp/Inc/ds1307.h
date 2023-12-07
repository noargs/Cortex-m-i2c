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
#define DS1307_I2C_SPEED              I2C_SCL_SPEED_XSM
#define DS1307_I2C_DEVICE_ADDR        0x68

#define DS1307_ADDR_SEC               0x00
#define DS1307_ADDR_MIN               0x01
#define DS1307_ADDR_HRS               0x02
#define DS1307_ADDR_DAY               0x03
#define DS1307_ADDR_DATE              0x04
#define DS1307_ADDR_MONTH             0x05
#define DS1307_ADDR_YEAR              0x06

#define SUNDAY                        1U;
#define MONDAY                        2U;
#define TUESDAY                       3U;
#define WEDNESDAY                     4U;
#define THURSDAY                      5U;
#define FRIDAY                        6U;
#define SATURDAY                      7U;

#define TIME_FORMAT_12HRS_AM          0U
#define TIME_FORMAT_12HRS_PM          1U
#define TIME_FORMAT_24HRS             2U


typedef struct
{
  uint8_t day;
  uint8_t date;
  uint8_t month;
  uint8_t year;
}rtc_date_t;

typedef struct
{
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t time_fmt;
}rtc_time_t;

uint8_t ds1307_init(void);

void ds1307_set_current_time(rtc_time_t *current_time);
void ds1307_get_current_time(rtc_time_t *current_time);
void ds1307_set_current_date(rtc_date_t *current_date);
void ds1307_get_current_date(rtc_date_t *current_date);



#endif /* DS1307_H_ */
