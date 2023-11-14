#include <stdio.h>
#include <stdint.h>
#include "ds1307.h"
#include "debug_uart.h"

char* time_to_string(rtc_time_t *rtc_time);
void number_to_string(uint8_t number, char* buf);

int main(void)
{
  DebugUART_Init();


  if (ds1307_init())
  {
	printf("DS1307 chip failed to boot... 'Error CH=1' \n\r");
	while (1);
  }

  printf("DS1307 chip intialised successfully \n\r");

  rtc_date_t current_date;
  rtc_time_t current_time;

  current_time.seconds   = 36;
  current_time.hours     = 7;
  current_time.minutes   = 41;
  current_time.time_fmt  = TIME_FORMAT_12HRS_AM;

  ds1307_set_current_time(&current_time);

  ds1307_get_current_time(&current_time);


  char *am_pm;

  if (current_time.time_fmt != TIME_FORMAT_24HRS)
  {
	printf("Hello from 12HRS \n\r");
	am_pm = (current_time.time_fmt) ? "PM" : "AM";
	printf("Current time: %s %s\n\r", time_to_string(&current_time), am_pm); // 07:41:36 AM
  }
  else
  {
	printf("Hello from 24HRS \n\r");
	printf("Current time: %s \n\r", time_to_string(&current_time)); // 07:41:36
  }


  while (1);

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
