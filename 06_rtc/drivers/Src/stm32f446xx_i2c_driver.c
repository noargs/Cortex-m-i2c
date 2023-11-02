#include "stm32f446xx_i2c_driver.h"

static uint32_t RCC_GetPCLK1Value(void);

uint16_t hpre[8]  = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t  ppre1[4] = {2, 4, 8, 16};

void I2C_Init (i2c_handle_t *i2c_handle)
{

  // Enable clock access to I2C1
  I2C1_PCLK_EN();

  // ACK only take into effect after PE=1

  uint32_t temp_reg = 0;

  // Configure the FREQ field CR2
  temp_reg |= (RCC_GetPCLK1Value() / 1000000U);
  i2c_handle->i2cx->CR2 = (temp_reg & 0x3F);

  // Program the device own address
  temp_reg = 0;
  temp_reg |= i2c_handle->i2c_config->i2c_device_address << 1U;
  temp_reg |= (0x1UL << (14U)); // always kept at 1 RM page: 784
  i2c_handle->i2cx->OAR1 = temp_reg;

  // CCR calculation
 uint16_t ccr = 0;
 temp_reg = 0;

 if (i2c_handle->i2c_config->i2c_scl_speed <= I2C_SCL_SPEED_SM)
 { // Standard Mode
   ccr = (RCC_GetPCLK1Value() / 2 * i2c_handle->i2c_config->i2c_scl_speed);
   temp_reg |= (ccr & 0xFFF);
 }
 else
 { // Fast mode
   temp_reg |= I2C_CCR_FS;
   temp_reg |= i2c_handle->i2c_config->i2c_fm_duty_cycle << I2C_CCR_DUTY_Pos;
   if (i2c_handle->i2c_config->i2c_fm_duty_cycle == I2C_FM_DUTY_2)
   {
	 ccr = (RCC_GetPCLK1Value() / 3 * i2c_handle->i2c_config->i2c_scl_speed);
   }
   else
   {
	 ccr = (RCC_GetPCLK1Value() / 25 * i2c_handle->i2c_config->i2c_scl_speed);
   }

   temp_reg |= (ccr & 0xFFF);
 }
 i2c_handle->i2cx->CCR = temp_reg;

 // Trise configuration
 if (i2c_handle->i2c_config->i2c_scl_speed <= I2C_SCL_SPEED_SM)
 {
   // Standard mode
   temp_reg = (RCC_GetPCLK1Value()/1000000U) + 1;
 }
 else
 {
   // Fast mode
   temp_reg = ((RCC_GetPCLK1Value() * 300) / 1000000000U) + 1;
 }
 i2c_handle->i2cx->TRISE = (temp_reg & 0x3F);

}

static uint32_t RCC_GetPCLK1Value(void)
{
  uint32_t system_clock;
  uint8_t clock_source, temp, ahb_prescaler, apb1_prescaler;

  // [SWS: System clock switch status] RM page: 133 (for Nucleo-f446re)
  clock_source = ((RCC->CFGR >> 2) & 0x3);

  if (clock_source == RCC_CFGR_SWS_HSI)
  {
	system_clock = 16000000;
  }
  else if (clock_source == RCC_CFGR_SWS_HSE)
  {
	system_clock = 8000000;
  }

  // [HPRE: AHB prescaler] RM page: 133 (for Nucleo-f446re)
  temp = ((RCC->CFGR >> 4) & 0xF);

  if (temp < 8)
  {
	ahb_prescaler = 1;
  }
  else
  {
	ahb_prescaler = hpre[temp-8];
  }

  // [PPRE1: APB Low speed prescaler (APB1)] RM page: 132 (for Nucleo-f446re)
  temp = ((RCC->CFGR >> 10) & 0x7);

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
