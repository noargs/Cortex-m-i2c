#include "stm32f446xx_i2c_driver.h"
#include "stm32f446xx_rcc_driver.h"

void I2C_Init (i2c_handle_t *i2c_handle)
{

  // Enable clock access to I2Cx
  I2C_PCLK_EN(i2c_handle->i2cx);

  i2c_handle->i2cx->CR1 &= ~I2C_CR1_PE;

  uint32_t temp_reg = 0;

  // ACK only take into effect after PE=1
//  i2c_handle->i2cx->CR1 |= I2C_CR1_ACK;
//  temp_reg |= i2c_handle->i2c_config.i2c_ack_control << I2C_CR1_ACK_Pos;
//  i2c_handle->i2cx->CR1 = temp_reg;

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
   ccr = (RCC_GetPCLK1Value() / (2 * i2c_handle->i2c_config.i2c_scl_speed));
   temp_reg |= (ccr & 0xFFF);
 }
 else
 { // Fast mode
   temp_reg |= I2C_CCR_FS;
   temp_reg |= i2c_handle->i2c_config.i2c_fm_duty_cycle << I2C_CCR_DUTY_Pos;
   if (i2c_handle->i2c_config.i2c_fm_duty_cycle == I2C_FM_DUTY_2)
   {
	 ccr = (RCC_GetPCLK1Value() / (3 * i2c_handle->i2c_config.i2c_scl_speed));
   }
   else
   {
	 ccr = (RCC_GetPCLK1Value() / (25 * i2c_handle->i2c_config.i2c_scl_speed));
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
  slave_address = slave_address << 1;
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
  slave_address = slave_address << 1;
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


void I2C_ClearBusyFlagErratum(gpio_handle_t* i2c_scl, gpio_handle_t* i2c_sda, i2c_handle_t *i2c_handle )
{
  // https://www.st.com/content/ccc/resource/technical/document/errata_sheet/7f/05/b0/bc/34/2f/4c/21/CD00288116.pdf/files/CD00288116.pdf/jcr:content/translations/en.CD00288116.pdf
  // [On Page: 19]
  //
  //1. Disable the I2C peripheral
  i2c_handle->i2cx->CR1 &= ~I2C_CR1_PE;

  //2. scl and sda i/o as general purpose output open-drain and high level (write 1 to GPIOx_ODR)
  i2c_scl->gpiox->MODER &= ~(0x3 << (2 * i2c_scl->gpio_config.pin_number));
  i2c_scl->gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_scl->gpio_config.pin_number));
  i2c_scl->gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_scl->gpio_config.pin_number);
  i2c_scl->gpiox->ODR |= (0x1UL << i2c_scl->gpio_config.pin_number);

  i2c_sda->gpiox->MODER &= ~(0x3 << (2 * i2c_sda->gpio_config.pin_number));
  i2c_sda->gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_sda->gpio_config.pin_number));
  i2c_sda->gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_sda->gpio_config.pin_number);
  i2c_sda->gpiox->ODR |= (0x1UL << i2c_sda->gpio_config.pin_number);

  //3. check scl and sda High level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_scl->gpiox, i2c_scl->gpio_config.pin_number) == RESET);
  while (GPIO_ReadFromInputPin(i2c_sda->gpiox, i2c_sda->gpio_config.pin_number) == RESET);

  //4. configure the sda i/o as general purpose output open-drain and low level (write 0 to GPIOx_ODR)
  i2c_sda->gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_sda->gpio_config.pin_number));
  i2c_sda->gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_sda->gpio_config.pin_number);
  i2c_sda->gpiox->ODR &= ~(0x1UL << i2c_sda->gpio_config.pin_number);

  //5. check sda low level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_sda->gpiox, i2c_sda->gpio_config.pin_number) != RESET);

  //6. configure the scl i/o as general purpose output open-drain and low level (write 0 to GPIOx_ODR)
  i2c_scl->gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_scl->gpio_config.pin_number));
  i2c_scl->gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_scl->gpio_config.pin_number);
  i2c_scl->gpiox->ODR &= ~(0x1UL << i2c_scl->gpio_config.pin_number);

  //7. check scl low level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_scl->gpiox, i2c_scl->gpio_config.pin_number) != RESET);

  //8. configure the scl i/o as general purpose output open-drain and high level (write 1 to GPIOx_ODR)
  i2c_scl->gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_scl->gpio_config.pin_number));
  i2c_scl->gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_scl->gpio_config.pin_number);
  i2c_scl->gpiox->ODR |= (0x1UL << i2c_scl->gpio_config.pin_number);

  //9. check scl High level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_scl->gpiox, i2c_scl->gpio_config.pin_number) == RESET);

  //10. configure the sda i/o as general purpose output open-drain and high level (write 1 to GPIOx_ODR)
  i2c_sda->gpiox->MODER |= (GPIO_MODE_OUT << (2 * i2c_sda->gpio_config.pin_number));
  i2c_sda->gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_sda->gpio_config.pin_number);
  i2c_sda->gpiox->ODR |= (0x1UL << i2c_sda->gpio_config.pin_number);

  //11. check sda High level in GPIOx_IDR
  while (GPIO_ReadFromInputPin(i2c_sda->gpiox, i2c_sda->gpio_config.pin_number) == RESET);

  //12. configure the scl and sda i/o's as Alternate function open-drain
  i2c_scl->gpiox->MODER &= ~(0x3 << (2 * i2c_scl->gpio_config.pin_number));
  i2c_scl->gpiox->MODER |= (GPIO_MODE_ALTFN << (2 * i2c_scl->gpio_config.pin_number));
  i2c_scl->gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_scl->gpio_config.pin_number);

  i2c_sda->gpiox->MODER &= ~(0x3 << (2 * i2c_sda->gpio_config.pin_number));
  i2c_sda->gpiox->MODER |= (GPIO_MODE_ALTFN << (2 * i2c_sda->gpio_config.pin_number));
  i2c_sda->gpiox->OTYPER |= (GPIO_OP_TYPE_OD << i2c_sda->gpio_config.pin_number);

  //13. set SWRST bit in I2Cx_CR1 register.
  i2c_handle->i2cx->CR1 |= I2C_CR1_SWRST;

  //14. clear SWRST bit in I2Cx_CR1 register.
  i2c_handle->i2cx->CR1 &= ~I2C_CR1_SWRST;

  //15. enable the I2C peripheral by setting PE bit in the I2Cx_CR1 register
  i2c_handle->i2cx->CR1 |= I2C_CR1_PE;
}
