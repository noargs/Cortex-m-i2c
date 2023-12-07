#include "i2c_drivers_f446xx.h"

uint16_t hpre[8] = {2,4,8,16,128,256,512};
uint8_t ppre[4] = {2,4,8,16};

static void I2C_PCLK_Enable(I2C_TypeDef *i2cx);

uint32_t RCC_GetPCLK1Value(void)
{
  uint32_t pclk1, system_clock;
  uint8_t clock_source, temp, ahb_prescaler, apb1_prescaler;

  clock_source = ((RCC->CFGR >> 2) & 0x3);
  if (clock_source == HSI_SOURCE)
  {
	system_clock = 16000000;
  }
  else if (clock_source == HSE_SOURCE)
  {
	system_clock = 8000000;
  }

  // find AHB Prescaler RM page: 228
  temp = ((RCC->CFGR >> 4) & 0xF);
  if (temp < 8)
  {
	ahb_prescaler = 1;
  }
  else
  {
	ahb_prescaler = hpre[temp-8];
  }

  // find APB1 Prescaler RM page: 229
  temp = ((RCC->CFGR >> 10) & 0xF);
  if (temp < 4)
  {
	apb1_prescaler = 1;
  }
  else
  {
	apb1_prescaler = ppre[temp-4];
  }

  return pclk1 = (system_clock/ahb_prescaler)/apb1_prescaler;
}

void I2C_Inits(I2C_TypeDef *i2cx)
{
  // Enable clock access
  I2C_PCLK_Enable(i2cx);

  // Configure ACK control bit CR1
  i2cx->CR1 |= I2C_CR1_ACK;

  // Configure the FREQ field CR2
  i2cx->CR2 |= ((RCC_GetPCLK1Value()/1000000U) & 0x3F);

  // device own address (slave address) OAR1 RM page:784
  i2cx->OAR1 |= (SLAVE_ADDRESS & I2C_OAR1_ADD1_7);

  // Reserved bit but always be kept at 1 by software RM page:784
  i2cx->OAR1 |= (0x1UL << (14U));


// CCR calculation
uint16_t ccr_value = 0;

#ifdef SPEED_STANDARD
  ccr_value = (RCC_GetPCLK1Value()/(2 * I2C_SCL_SPEED_SM));
  i2cx->CCR |= (ccr_value & 0xFFF);
#else // Speed fast (MUST `#define DUTY_ZERO` or leave it off for `Duty cycle 1`)
  i2cx->CCR |= I2C_CCR_FS;
#ifdef DUTY_ZERO
  i2cx->CCR &= ~I2C_CCR_DUTY;
  ccr_value = (RCC_GetPCLK1Value() / 3 * I2C_SCL_SPEED_FM4K);
  i2cx->CCR |= (ccr_value & 0xFFF);
#else // Duty cycle 1
  i2cx->CCR |= I2C_CCR_DUTY;
  ccr_value = (RCC_GetPCLK1Value() / 25 * I2C_SCL_SPEED_FM4K);
  i2cx->CCR |= (ccr_value & 0xFFF);
#endif
#endif

// TRISE configuration
uint32_t trise = 0;
#ifdef SPEED_STANDARD
  trise = (RCC_GetPCLK1Value()/1000000U) + 1;     // Fpclk (8Mhz) x Trise (1000ns or 1 us) / (1000000 to convert 8Mhz to ns as f = 1/t)
#else
  trise = ((RCC_GetPCLK1Value() * 300) / 1000000000U) + 1; // 1MHz or 1xe10 not sure :`(
#endif
  i2cx->TRISE = (trise & 0x3F);
}


void I2C_MasterSendData(I2C_TypeDef *i2cx, uint8_t *tx_buffer, uint32_t len, uint8_t slave_addr)
{
  //1. Generate the START condition
  i2cx->CR1 |= I2C_CR1_START;

  //2. Confirm start generation (i.e. check SB flag in SR1, SB
  //   stretches the SCL low until cleared)
  while ( (i2cx->SR1 & I2C_SR1_SB) == RESET);

  //3. Send slave address with r/w bit as 0 i.e. write (slave addr 7bits + r/w 1bit)
  slave_addr = slave_addr << 1;
  slave_addr &= ~(1);
  i2cx->DR = slave_addr;

  //4. Confirm address phase completed (i.e. check ADDR flag in SR1)
  while ( (i2cx->SR1 & I2C_SR1_ADDR) == RESET);


  //5. Clear the ADDR with software sequence (read SR1 and then SR2, ADDR
  //   stretches the SCL low until cleared)
  uint32_t dummy_read = i2cx->SR1;
  dummy_read = i2cx->SR2;
  (void)dummy_read;

  //6. Send the data until `len` becomes `0`
  while (len > 0)
  {
	while ( (i2cx->SR1 & I2C_SR1_TXE) == RESET);
	i2cx->DR = *tx_buffer;
	tx_buffer++;
	len--;
  }

  //7. When `len=0` wait for TxE=1, BTF=1 before generating STOP condition
  //   TxE=1, BTF=1 means both SR and DR are empty and next transmission should begin
  //   BTF=1 also stretches the SCL to low
  while ( (i2cx->SR1 & I2C_SR1_TXE) == RESET);
  while ( (i2cx->SR1 & I2C_SR1_BTF) == RESET);

  //8. Generate STOP condition and Master should wait for the completion of STOP condition
  //   Generating STOP, auto clears the BTF
  i2cx->CR1 |= I2C_CR1_STOP;
}


static void I2C_PCLK_Enable(I2C_TypeDef *i2cx)
{
  if (i2cx == I2C1)
  {
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
  }
  else if (i2cx == I2C2)
  {
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
  }
  else if (i2cx == I2C3)
  {
	RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
  }
}

uint8_t GPIO_ReadFromInputPin(GPIO_TypeDef *gpiox, uint8_t pin_number)
{
  return (uint8_t)((gpiox->IDR >> pin_number) & 0x00000001);
}



