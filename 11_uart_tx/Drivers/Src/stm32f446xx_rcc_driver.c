#include "stm32f446xx_rcc_driver.h"

uint16_t hpre[8]  = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t  ppre1[4] = {2, 4, 8, 16};
uint8_t  ppre2[4] = {2, 4, 8, 16};

uint32_t RCC_GetPCLK1Value(void)
{
  uint32_t system_clock;
  uint8_t clock_source, temp, ahb_prescaler, apb1_prescaler;

  // [SWS: System clock switch status] RM page: 133 (for Nucleo-f446re)
  clock_source = ((RCC->CFGR >> RCC_CFGR_SWS_Pos) & 0x3);

  if (clock_source == SYSTEM_CLOCK_SWS_HSI)
  {
	system_clock = 16000000;
  }
  else if (clock_source == SYSTEM_CLOCK_SWS_HSE)
  {
	system_clock = 8000000;
  }

  // [HPRE: AHB prescaler] RM page: 133 (for Nucleo-f446re)
  temp = ((RCC->CFGR >> RCC_CFGR_HPRE_Pos) & 0xF);

  if (temp < 8)
  {
	ahb_prescaler = 1;
  }
  else
  {
	ahb_prescaler = hpre[temp-8];
  }

  // [PPRE1: APB Low speed prescaler (APB1)] RM page: 132 (for Nucleo-f446re)
  temp = ((RCC->CFGR >> RCC_CFGR_PPRE1_Pos) & 0x7);

  if (temp < 4)
  {
	apb1_prescaler = 1;
  }
  else
  {
	apb1_prescaler = ppre1[temp-4];
  }

  return (uint32_t)((system_clock/ahb_prescaler)/apb1_prescaler);

}

uint32_t RCC_GetPCLK2Value(void)
{
  uint32_t system_clock =0, temp;

  uint8_t ahb_prescaler, apb2_prescaler;

  // [SWS: System clock switch status] RM page: 133 (for Nucleo-f446re)
  uint32_t clock_source = ((RCC->CFGR >> RCC_CFGR_SWS_Pos) & 0x3);

  if (clock_source == SYSTEM_CLOCK_SWS_HSI)
  {
	system_clock = 16000000;
  }
  else if (clock_source == SYSTEM_CLOCK_SWS_HSE)
  {
	system_clock = 8000000;
  }

  // [HPRE: AHB prescaler] RM page: 133 (for Nucleo-f446re)
  temp = ((RCC->CFGR >> RCC_CFGR_HPRE_Pos) & 0xF);

  if (temp < 0x08)
  {
	ahb_prescaler = 1;
  }
  else
  {
	ahb_prescaler = hpre[temp-8];
  }

  temp = ((RCC->CFGR >> RCC_CFGR_PPRE2_Pos) & 0x7);

  if (temp < 0x04)
  {
	apb2_prescaler = 1;
  }
  else
  {
	apb2_prescaler = ppre2[temp-4];
  }
  return (uint32_t)((system_clock/ahb_prescaler)/apb2_prescaler);
}

uint32_t RCC_GetPLLCLKValue(void)
{
  return 0;
}
