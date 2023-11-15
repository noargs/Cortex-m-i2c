#include <stdio.h>
#include <stdint.h>
#include "ds1307.h"
#include "debug_uart.h"

#define SYSTICK_TIM_CLK               16000000UL

#define CSR_ENABLE                    (1 << 0U)    // SYST_CSR [GUM page: 249], enable the counter
#define CSR_TICKINT                   (1 << 1U)    // Enable SysTick exception request
#define CSR_CLKSRC                    (1 << 2U)    // ...,  0=external clock, 1=processor clock

char* time_to_string(rtc_time_t *rtc_time);
void number_to_string(uint8_t number, char* buf);
char* date_to_string(rtc_date_t *current_date);
char* get_day_of_week(uint8_t day);
void init_date_time(void);
void output_date_time(void);
void systick_init(uint32_t tick_hz);

rtc_date_t current_date;
rtc_time_t current_time;

int main(void)
{
  DebugUART_Init();


  if (ds1307_init())
  {
	printf("\rDS1307 initialisation failed, cpu halt ch=1\n\r");
	while (1);
  }

  systick_init(1);

  printf("\rDS1307 intialised successfully\n");

  init_date_time();

  output_date_time();

  while (1);

}

void SysTick_Handler (void)
{
  output_date_time();
}

void init_date_time(void)
{
  current_date.day       = TUESDAY;
  current_date.date      = 14;
  current_date.month     = 11;
  current_date.year      = 23;

  current_time.seconds   = 10;
  current_time.hours     = 4;
  current_time.minutes   = 14;
  current_time.time_fmt  = TIME_FORMAT_12HRS_AM;

  ds1307_set_current_date(&current_date);
  ds1307_set_current_time(&current_time);
}

void output_date_time(void)
{
  ds1307_get_current_date(&current_date);
  ds1307_get_current_time(&current_time);


  char *am_pm;

  if (current_time.time_fmt != TIME_FORMAT_24HRS)
  {
	am_pm = (current_time.time_fmt) ? "AM" : "PM";
	printf("%s %s - ", time_to_string(&current_time), am_pm); // 07:41:36 AM
  }
  else
  {
	printf("%s - ", time_to_string(&current_time)); // 07:41:36
  }

  printf("%s %s \n\r", date_to_string(&current_date), get_day_of_week(current_date.day));
}

void systick_init(uint32_t tick_hz)
{
  // Systick Register names in header file are different
  // SysTick->LOAD corresponse to `SysTick Reload Value Register` [SYST_RVR]
  // SysTick->VAL    ==>   `SysTick Current Value Register` [SYST_CVR]
  // SysTick->CTRL   ==>   `SysTick Control and Status Register` [SYST_CSR]

  // 1. Reload with number of clocks per milliseconds [SYST_RVR] [GUM page: 249]
  SysTick->LOAD = (SYSTICK_TIM_CLK/tick_hz) - 1; // for 16000 clock pulses, set RELOAD 15999 (i.e. N-1)

  // 2. Clear `Systick current value register` [SYST_CVR] [GUM page: 249]
  SysTick->VAL = 0;

  // 3. Choose clock source, enable SysTick exception request, and enable the counter
  SysTick->CTRL = CSR_CLKSRC | CSR_TICKINT | CSR_ENABLE;
}

// hh:mm:ss
char* time_to_string(rtc_time_t *rtc_time)
{
  static char buf[9];
  buf[2]=':';
  buf[5]=':';
  number_to_string(rtc_time->hours, buf);
  number_to_string(rtc_time->minutes, &buf[3]);
  number_to_string(rtc_time->seconds, &buf[6]);
  buf[8] = '\0';
  return buf;
}

void number_to_string(uint8_t number, char* buf)
{
  if (number < 10)
  {
	buf[0] = '0';
	buf[1] = number + 48; // + 48 to convert to ASCII
  }
  else if (number >= 10 && number < 99)
  {
	buf[0] = (number/10) + 48;
	buf[1] = (number%10) + 48;
  }
}

// dd-mm-yy
char* date_to_string(rtc_date_t *current_date)
{
  static char buf[9];
  buf[2]='-';
  buf[5]='-';
  number_to_string(current_date->date, buf);
  number_to_string(current_date->month, &buf[3]);
  number_to_string(current_date->year, &buf[6]);
  buf[8]='\0';
  return buf;
}

char* get_day_of_week(uint8_t day)
{
  char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"};
  return weekdays[day-1];
}
