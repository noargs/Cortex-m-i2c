#include "stm32f446xx_i2c_driver.h"

static uint32_t RCC_GetPCLK1Value(void);

uint16_t hpre[8]  = {2, 4, 8, 16, 64, 128, 256, 512};
uint8_t  ppre1[4] = {2, 4, 8, 16};

void I2C_Init (i2c_handle_t *i2c_handle)
{

  // Enable clock access to I2C1
  I2C_PCLK_EN(i2c_handle->i2cx);

  // ACK only take into effect after PE=1
  i2c_handle->i2cx->CR1 |= I2C_CR1_ACK;

  uint32_t temp_reg = 0;

  // [ Configure the FREQ field CR2 ]
  temp_reg |= (RCC_GetPCLK1Value() / 1000000U);
  i2c_handle->i2cx->CR2 = (temp_reg & 0x3F);

  // Program the device own address
  temp_reg = 0;
  temp_reg |= i2c_handle->i2c_config.i2c_device_address << 1U;
  temp_reg |= (0x1UL << (14U)); // always kept at 1 RM page: 784
  i2c_handle->i2cx->OAR1 = temp_reg;

  // [ CCR calculation ]
 uint16_t ccr = 0;
 temp_reg = 0;

 if (i2c_handle->i2c_config.i2c_scl_speed <= I2C_SCL_SPEED_SM)
 { // Standard Mode
   ccr = (RCC_GetPCLK1Value() / 2 * i2c_handle->i2c_config.i2c_scl_speed);
   temp_reg |= (ccr & 0xFFF);
 }
 else
 { // Fast mode
   temp_reg |= I2C_CCR_FS;
   temp_reg |= i2c_handle->i2c_config.i2c_fm_duty_cycle << I2C_CCR_DUTY_Pos;
   if (i2c_handle->i2c_config.i2c_fm_duty_cycle == I2C_FM_DUTY_2)
   {
	 ccr = (RCC_GetPCLK1Value() / 3 * i2c_handle->i2c_config.i2c_scl_speed);
   }
   else
   {
	 ccr = (RCC_GetPCLK1Value() / 25 * i2c_handle->i2c_config.i2c_scl_speed);
   }

   temp_reg |= (ccr & 0xFFF);
 }
 i2c_handle->i2cx->CCR = temp_reg;

 // [ Trise configuration ]
 if (i2c_handle->i2c_config.i2c_scl_speed <= I2C_SCL_SPEED_SM)
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


void I2C_MasterSendData(i2c_handle_t *i2c_handle, uint8_t *tx_buffer, uint32_t length, uint8_t slave_address, uint8_t sr_repeated_start )
{
  //1. Generate the START condition
  i2c_handle->i2cx->CR1 |= I2C_CR1_START;

  //2. Confirm start generation (i.e. check SB flag in SR1, SB
  //   stretches the SCL low until cleared)
  while ((i2c_handle->i2cx->SR1 & I2C_SR1_SB) == RESET);

  //3. Send slave address with r/w bit as 0 i.e. write (slave addr 7bits + r/w 1bit)
  slave_address = slave_address < 1;
  slave_address &= ~(1);
  i2c_handle->i2cx->DR = slave_address;

  //4. Confirm address phase completed (i.e. check ADDR flag in SR1)
  while ((i2c_handle->i2cx->SR1 & I2C_SR1_ADDR) == RESET);

  //5. Clear the ADDR with software sequence (read SR1 and then SR2, ADDR
  //   stretches the SCL low until cleared)
  uint32_t dummy_read = i2c_handle->i2cx->SR1;
  dummy_read = i2c_handle->i2cx->SR2;
  (void)dummy_read;

  while (length > 0)
  {
	while ((i2c_handle->i2cx->SR1 & I2C_SR1_TXE) == RESET);
	i2c_handle->i2cx->DR = *tx_buffer;
	tx_buffer++;
	length--;
  }

  //7. When `len=0` wait for TxE=1, BTF=1 before generating STOP condition
  //   TxE=1, BTF=1 means both SR and DR are empty and next transmission should begin
  //   BTF=1 also stretches the SCL to low
  while ((i2c_handle->i2cx->SR1 & I2C_SR1_TXE) == RESET);
  while ((i2c_handle->i2cx->SR1 & I2C_SR1_BTF) == RESET);

  //8. Generate STOP condition and Master should wait for the completion of STOP condition
  //   Generating STOP, auto clears the BTF
  if (sr_repeated_start == I2C_SR_DISABLE)
  {
	i2c_handle->i2cx->CR1 |= I2C_CR1_STOP;
  }

}


void I2C_MasterReceiveData(i2c_handle_t *i2c_handle, uint8_t *rx_buffer, uint32_t length, uint8_t slave_address, uint8_t sr_repeated_start)
{
  //1. Generate the START condition
  i2c_handle->i2cx->CR1 |= I2C_CR1_START;

  //2. Confirm that Start generation is completed by checking the SB flag in the SR
  // Note: Until SB is cleared SCL will be stretched (pulled to LOW)
  while ((i2c_handle->i2cx->SR1 & I2C_SR1_SB) == RESET);

  //3. Send the address of the slave with R/W bit set to 1 (i.e. R) (total 8 bits)
  slave_address = slave_address < 1;
  slave_address |= 1;
  i2c_handle->i2cx->DR = slave_address;

  //4. Wait until Address phase is completed by checking the ADDR flag in the SR1
  while ((i2c_handle->i2cx->SR1 & I2C_SR1_ADDR) == RESET);


  // ...... Procedure to read only [1 byte] from the Slave ........
  if (length == 1)
  {
	// Disable Ack
	i2c_handle->i2cx->CR1 &= ~I2C_CR1_ACK;

	// Clear the ADDR flag
	uint32_t dummy_read = i2c_handle->i2cx->SR1;
	dummy_read = i2c_handle->i2cx->SR2;
	(void)dummy_read;

	// Wait untill RxNE becomes 1
	while ((i2c_handle->i2cx->SR1 & I2C_SR1_RXNE) == RESET);

	// Generate STOP condition
	if (sr_repeated_start == I2C_SR_DISABLE)
	{
	  i2c_handle->i2cx->CR1 |= I2C_CR1_STOP;
	}

	// Read data into the buffer
	*rx_buffer = i2c_handle->i2cx->DR;
  }

  // ...... Procedure to read more than 1 byte [len > 1] from the Slave ........
  if (length > 1)
  {
	// Clear the ADDR flag
	uint32_t dummy_read = i2c_handle->i2cx->SR1;
	dummy_read = i2c_handle->i2cx->SR2;
	(void)dummy_read;

	for (uint32_t i=length; i>0; i--)
	{
	  // Wait until RxNE becomes 1
	  while ((i2c_handle->i2cx->SR1 & I2C_SR1_RXNE) == RESET);

	  if (i==2)
	  {
		// Clear the ACK bit
		i2c_handle->i2cx->CR1 &= ~I2C_CR1_ACK;

		// Generate STOP condition
		if (sr_repeated_start == I2C_SR_DISABLE)
		{
		  i2c_handle->i2cx->CR1 |= I2C_CR1_STOP;
		}
	  }


	  // Read the data from DR in to buffer AND increment the buffer address
	  *rx_buffer = i2c_handle->i2cx->DR;
	  rx_buffer++;
	}
  }
  // Re-enable ACK
  i2c_handle->i2cx->CR1 |= I2C_CR1_ACK;
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
