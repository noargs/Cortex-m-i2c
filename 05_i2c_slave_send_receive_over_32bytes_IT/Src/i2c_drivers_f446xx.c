#include <stdint.h>
#include "i2c_drivers_f446xx.h"

uint16_t hpre[8] = {2,4,8,16,128,256,512};
uint8_t ppre[4] = {2,4,8,16};

static void I2C_MasterHandleTxeIT(i2c_handle_t *i2c_handle);
static void I2C_MasterHandleRxneIT(i2c_handle_t *i2c_handle);
static void I2C_ClearAddrFlag(i2c_handle_t *i2c_handle);
static void I2C_ReadStatusRegisters(i2c_handle_t *i2c_handle);
//static void I2C_CloseSendData(i2c_handle_t *i2c_handle);
static void I2C_CloseReceiveData(i2c_handle_t *i2c_handle);
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
  temp = ((RCC->CFGR >> 10) & 0x7);
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
  // Only take into effect after PE=1, hence enable ACK after PE is enabled already
  //i2cx->CR1 |= I2C_CR1_ACK;

  // Configure the FREQ field CR2
  i2cx->CR2 |= ((RCC_GetPCLK1Value()/1000000U) & 0x3F);

  // device own address (slave address) OAR1 RM page:784
  i2cx->OAR1 |= (SLAVE_ADDRESS << 1);

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
  trise = ((RCC_GetPCLK1Value() * 300) / 1000000000U) + 1; // convert to Nano
#endif
  i2cx->TRISE = (trise & 0x3F);
}


void I2C_MasterSendData(I2C_TypeDef *i2cx, uint8_t *tx_buffer, uint32_t len, uint8_t slave_addr, uint8_t repeated_start)
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
  if (repeated_start == I2C_DISABLE_SR)
	i2cx->CR1 |= I2C_CR1_STOP;
}

void I2C_MasterReceiveData(I2C_TypeDef *i2cx, uint8_t *rx_buffer, uint8_t len, uint8_t slave_addr, uint8_t repeated_start)
{
  //1. Generate the START condition
  i2cx->CR1 |= I2C_CR1_START;

  //2. Confirm that Start generation is completed by checking the SB flag in the SR
  // Note: Until SB is cleared SCL will be stretched (pulled to LOW)
  while ( (i2cx->SR1 & I2C_SR1_SB) == RESET);

  //3. Send the address of the slave with R/W bit set to 1 (i.e. R) (total 8 bits)
  slave_addr = slave_addr << 1;
  slave_addr |= 1;
  i2cx->DR = slave_addr;

  //4. Wait until Address phase is completed by checking the ADDR flag in the SR1
  while ( (i2cx->SR1 & I2C_SR1_ADDR) == RESET);

  // Procedure to read only 1 byte from the Slave
  if (len == 1) {
    // Disable Acking
	i2cx->CR1 &= ~I2C_CR1_ACK;

    // Clear the ADDR flag
	uint32_t dummy_read = i2cx->SR1;
	dummy_read = i2cx->SR2;
	(void)dummy_read;

    // Wait until RxNE becomes 1
	while ( (i2cx->SR1 & I2C_SR1_RXNE) == RESET);

    // Generate STOP condition
	if (repeated_start == I2C_DISABLE_SR)
		i2cx->CR1 |= I2C_CR1_STOP;

    // Read data into the buffer
	*rx_buffer = i2cx->DR;
  }

  // Procedure to read data from Slave when len > 1
  if (len > 1) {
	// Clear the ADDR flag
	uint32_t dummy_read = i2cx->SR1;
	dummy_read = i2cx->SR2;
	(void)dummy_read;

	// Read the data until len becomes 0
	for (uint32_t i=len; i>0; i--) {

	  // Wait until RxNE becomes 1
	  while ( (i2cx->SR1 & I2C_SR1_RXNE) == RESET);

	  if (i == 2) // if last 2 bytes are remaining
	  {
		// Clear the ACK bit
		i2cx->CR1 &= ~I2C_CR1_ACK;

		// Generate STOP condition
		if (repeated_start == I2C_DISABLE_SR)
			i2cx->CR1 |= I2C_CR1_STOP;
	  }

	  // Read the data from DR in to buffer AND Increment the buffer address
	  *rx_buffer = i2cx->DR;
	  rx_buffer++;
	}
  }
  // Re-enable ACK
  i2cx->CR1 |= I2C_CR1_ACK;
}

void I2C_SlaveSendData(I2C_TypeDef *i2cx, uint8_t data)
{
  i2cx->DR = data;
}


uint8_t I2C_SlaveReceiveData(I2C_TypeDef *i2cx)
{
  return (uint8_t)i2cx->DR;
}


uint8_t I2C_MasterSendDataInterrupt (i2c_handle_t *i2c_handle, uint8_t *tx_buffer,
									 uint32_t length, uint8_t slave_address, uint8_t repeated_start)
{
  uint8_t busy_state = i2c_handle->tx_rx_state;
  if ((busy_state != I2C_BUSY_IN_TX) && (busy_state != I2C_BUSY_IN_RX))
  {
	i2c_handle->tx_buffer = tx_buffer;
	i2c_handle->tx_length = length;
	i2c_handle->tx_rx_state = I2C_BUSY_IN_TX;
	i2c_handle->device_address = slave_address;
	i2c_handle->repeated_start_sr = repeated_start;

	// Generate START condition
	i2c_handle->i2cx->CR1 |= I2C_CR1_START;

	// Enable ITBUFEN control bit
	i2c_handle->i2cx->CR2 |= I2C_CR2_ITBUFEN;

	// Enable ITEVTEN control bit
	i2c_handle->i2cx->CR2 |= I2C_CR2_ITEVTEN;

	// Enable ITERREN control bit
	i2c_handle->i2cx->CR2 |= I2C_CR2_ITERREN;
  }
  return busy_state;
}


uint8_t I2C_MasterReceiveDataInterrupt (i2c_handle_t *i2c_handle, uint8_t *rx_buffer,
									    uint32_t length, uint8_t slave_address, uint8_t repeated_start)
{
  uint8_t busy_state = i2c_handle->tx_rx_state;
  if ((busy_state != I2C_BUSY_IN_TX) && (busy_state != I2C_BUSY_IN_RX))
  {
	i2c_handle->rx_buffer = rx_buffer;
	i2c_handle->rx_length = length;
	i2c_handle->tx_rx_state = I2C_BUSY_IN_RX;
	i2c_handle->rx_size = length;
	i2c_handle->device_address = slave_address;
	i2c_handle->repeated_start_sr = repeated_start;

	// Generate START Condition
	i2c_handle->i2cx->CR1 |= I2C_CR1_START;

	// Enable ITBUFEN control bit
	i2c_handle->i2cx->CR2 |= I2C_CR2_ITBUFEN;

	// Enable ITEVTEN control bit
	i2c_handle->i2cx->CR2 |= I2C_CR2_ITEVTEN;

	// Enable ITERREN control bit
	i2c_handle->i2cx->CR2 |= I2C_CR2_ITERREN;
  }
  return busy_state;
}


void I2C_EV_IRQHandling (i2c_handle_t *i2c_handle)
{
  // Interrupt handling of Master and Slave mode
  uint32_t itevten, itbufen, c;
  itevten = i2c_handle->i2cx->CR2 & I2C_CR2_ITEVTEN;
  itbufen = i2c_handle->i2cx->CR2 & I2C_CR2_ITBUFEN;

  // 1. Handle `SB event` interrupt (only applicable in Master mode, For Slave SB=0 always)
  c = i2c_handle->i2cx->SR1 & I2C_SR1_SB;
  if (itevten && c)
  {
	uint8_t slave_address;

	// SB flag is set
	if (i2c_handle->tx_rx_state == I2C_BUSY_IN_TX)
	{
	  slave_address = SLAVE_ADDRESS << 1;
	  slave_address &= ~(1);
	  i2c_handle->i2cx->DR = slave_address;
	} else if (i2c_handle->tx_rx_state == I2C_BUSY_IN_RX)
	{
	  slave_address = SLAVE_ADDRESS << 1;
	  slave_address |= 1;
	  i2c_handle->i2cx->DR = slave_address;
	}
  }

  // 2. Handle `ADDR event` interrupt
  //    [Note] In Master mode: Address is sent
  //           In Slave mode:  Address is matched with its own address
  c = i2c_handle->i2cx->SR1 & I2C_SR1_ADDR;
  if (itevten && c)
  {
	// ADDR flag is set
	I2C_ClearAddrFlag(i2c_handle);
  }

  // 3. Handle `BTF event` (Byte transfer finished) interrupt
  c = i2c_handle->i2cx->SR1 & I2C_SR1_BTF;
  if (itevten && c)
  {
	// BTF flag is set
	if (i2c_handle->tx_rx_state == I2C_BUSY_IN_TX)
	{
	  // make sure TXE is also set
	  if (i2c_handle->i2cx->SR1 & I2C_SR1_TXE)
	  {
		if (i2c_handle->tx_length == 0)
		{
		  //1. generate STOP condition
		  if (i2c_handle->repeated_start_sr == I2C_DISABLE_SR)
			i2c_handle->i2cx->CR1 |= I2C_CR1_STOP;

		  //2. reset all the member elements of `i2c_handle_t`
		  I2C_CloseSendData(i2c_handle);

		  //3. notify the application about transmission complete
		  I2C_ApplicationEventCallback(i2c_handle, I2C_EV_TX_COMPLETE);
		}
	  }
	}
	else if (i2c_handle->tx_rx_state == I2C_BUSY_IN_RX)
	{

	}
  }

  // 4. Handle `STOPF event` interrupt (only applicable in Slave Receiver mode)
  c = i2c_handle->i2cx->SR1 & I2C_SR1_STOPF;
  if (itevten && c)
  {
	// STOPF flag is set
	// Clear the STOPF (1. read SR1, 2. Write to CR1)
	uint32_t dummy_read = i2c_handle->i2cx->SR1;
	(void) dummy_read;
	i2c_handle->i2cx->CR1 |= 0x0000;

	I2C_ApplicationEventCallback(i2c_handle, I2C_EV_STOP);
  }

  // 5. Handle `TXE event` interrupt
  c = i2c_handle->i2cx->SR1 & I2C_SR1_TXE;
  if (itevten && itbufen && c)
  {
	// TXE flag is set
	if (i2c_handle->i2cx->SR2 & I2C_SR2_MSL)
	{
	  if (i2c_handle->tx_rx_state == I2C_BUSY_IN_TX)
		I2C_MasterHandleTxeIT(i2c_handle);
	}
	else
	{
	  // The device is Slave (and also Slave transmitter as checking TXE)
	  // Transmission bit TRA=1 means Slave in Transmitter mode [Reference Manual page:869]
	  if (i2c_handle->i2cx->SR2 & I2C_SR2_TRA)
		I2C_ApplicationEventCallback(i2c_handle, I2C_EV_DATA_REQUEST);
	}
  }

  // 6. Handle `RXNE event` interrupt
  c = i2c_handle->i2cx->SR1 & I2C_SR1_RXNE;
  if (itevten && itbufen && c)
  {
	// RXNE flag is set
	if (i2c_handle->i2cx->SR2 & I2C_SR2_MSL)
	{
	  if (i2c_handle->tx_rx_state == I2C_BUSY_IN_RX)
		I2C_MasterHandleRxneIT(i2c_handle);
	}
	else
	{
	  // Transmission bit TRA=0 means Slave in Receiver mode [RM page:869]
	  if (! (i2c_handle->i2cx->SR2 & I2C_SR2_TRA))
		I2C_ApplicationEventCallback(i2c_handle, I2C_EV_DATA_RECEIVE);
	}
  }
}

void I2C_ERR_IRQHandling (i2c_handle_t *i2c_handle)
{
  uint32_t iterren, error;

  iterren = i2c_handle->i2cx->CR2 & I2C_CR2_ITERREN;

  // Check for `Bus error`
  error = i2c_handle->i2cx->SR1 & I2C_SR1_BERR;
  if (error && iterren)
  {
	// Clear the Bus error flag and notify the application
	i2c_handle->i2cx->SR1 &= ~I2C_SR1_BERR;
	I2C_ApplicationEventCallback(i2c_handle, I2C_ERROR_BERR);
  }

  // Check for `Arbitration lost error`
  error = i2c_handle->i2cx->SR1 & I2C_SR1_ARLO;
  if (error && iterren)
  {
	// Clear the Arbitration lost error flag and notify the application
	i2c_handle->i2cx->SR1 &= ~I2C_SR1_ARLO;
	I2C_ApplicationEventCallback(i2c_handle, I2C_ERROR_ARLO);
  }

  // Check for `ACK Failure error`
  error = i2c_handle->i2cx->SR1 & I2C_SR1_AF;
  if (error && iterren)
  {
	// Clear the AF error flag and notify the application
	i2c_handle->i2cx->SR1 &= ~I2C_SR1_AF;
	I2C_ApplicationEventCallback(i2c_handle, I2C_ERROR_AF);
  }

  // Check for `Overrun error`
  error = i2c_handle->i2cx->SR1 & I2C_SR1_OVR;
  if (error && iterren)
  {
	// Clear the Overrun error flag and notify the application
	i2c_handle->i2cx->SR1 &= ~I2C_SR1_OVR;
	I2C_ApplicationEventCallback(i2c_handle, I2C_ERROR_OVR);
  }

  // Check for `Timeout error`
  error = i2c_handle->i2cx->SR1 & I2C_SR1_TIMEOUT;
  if (error && iterren)
  {
	// Clear the Timeout error flag and notify the application
	i2c_handle->i2cx->SR1 &= ~I2C_SR1_TIMEOUT;
	I2C_ApplicationEventCallback(i2c_handle, I2C_ERROR_TIMEOUT);
  }
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


void I2C_IRQInterruptConfig(uint8_t irq_number, uint8_t enable_or_disable)
{
  if (enable_or_disable == ENABLE)
  {
	if (irq_number <= 31)
	{
	  *NVIC_ISER0 |= (1 << irq_number);
	} else if (irq_number > 31 && irq_number < 64)
	{
	  *NVIC_ISER1 |= (1 << (irq_number % 32));
	} else if (irq_number >= 64 && irq_number < 96)
	{
	  *NVIC_ISER2 |= (1 << (irq_number % 64));
	}
  }
  else
  {
	if (irq_number <= 31)
	{
	  *NVIC_ICER0 |= (1 << irq_number);
	} else if (irq_number > 31 && irq_number < 64)
	{
	  *NVIC_ICER1 |= (1 << (irq_number % 32));
	} else if (irq_number >= 64 && irq_number < 96)
	{
	  *NVIC_ICER2 |= (1 << (irq_number % 64));
	}
  }
}


void I2C_IRQPriorityConfig(uint8_t irq_number, uint32_t irq_priority)
{
  uint8_t iprx = irq_number / 4;
  uint8_t iprx_section = irq_number % 4;
  uint8_t shift_amount = (8*iprx_section) + (8-NO_PR_BITS_IMPLEMENTED);
  *(NVIC_PR_BASE_ADDR + iprx) |= (irq_priority << shift_amount);
}

static void I2C_MasterHandleTxeIT(i2c_handle_t *i2c_handle)
{
  if (i2c_handle->tx_length > 0)
  {
	i2c_handle->i2cx->DR = *(i2c_handle->tx_buffer);
	i2c_handle->tx_length--;
	i2c_handle->tx_buffer++;
  }
}

static void I2C_MasterHandleRxneIT(i2c_handle_t *i2c_handle)
{
  if (i2c_handle->rx_size == 1)
  {
	*i2c_handle->rx_buffer = i2c_handle->i2cx->DR;
	i2c_handle->rx_length--;
  }

  if (i2c_handle->rx_size > 1)
  {
	if (i2c_handle->rx_length == 2)
	{
	  // Clear ACK bit
	  i2c_handle->i2cx->CR1 &= ~I2C_CR1_ACK;
	}
	// Read DR
	*i2c_handle->rx_buffer = i2c_handle->i2cx->DR;
	i2c_handle->rx_buffer++;
	i2c_handle->rx_length--;
  }

  if (i2c_handle->rx_length == 0)
  {
	//1. generate the STOP condition
	if (i2c_handle->repeated_start_sr == I2C_DISABLE_SR)
	  i2c_handle->i2cx->CR1 |= I2C_CR1_STOP;

	//2. close the I2C RX
	I2C_CloseReceiveData(i2c_handle);

	//3. notify the application
	I2C_ApplicationEventCallback(i2c_handle, I2C_EV_RX_COMPLETE);
  }

}

static void I2C_ClearAddrFlag(i2c_handle_t *i2c_handle)
{
	// check for device mode
	if (i2c_handle->i2cx->SR2 & I2C_SR2_MSL)
	{
	  // device mode: Master
	  if (i2c_handle->tx_rx_state == I2C_BUSY_IN_RX)
	  {
		if (i2c_handle->rx_size == 1)
		{
		  // First disable the ACK then clear the ADDR flag by reading SR1 and SR2
		  i2c_handle->i2cx->CR1 &= ~I2C_CR1_ACK;
		  I2C_ReadStatusRegisters(i2c_handle);
		}
		else
		{
		  // device mode: Master, I2C_BUSY_IN_TX - clear ADDR flag by reading SR1 and SR2
		  I2C_ReadStatusRegisters(i2c_handle);
		}
	  }
	}
	else
	{
	  // device mode: Slave
	  I2C_ReadStatusRegisters(i2c_handle);
	}
}

static void I2C_ReadStatusRegisters(i2c_handle_t *i2c_handle)
{
  uint32_t dummy_read;
  dummy_read = i2c_handle->i2cx->SR1;
  dummy_read = i2c_handle->i2cx->SR2;
  (void)dummy_read;
}

void I2C_CloseSendData(i2c_handle_t *i2c_handle)
{
  // disable ITBUFEN control bit
  i2c_handle->i2cx->CR2 &= ~I2C_CR2_ITBUFEN;

  // disable ITEVTEN control bit
  i2c_handle->i2cx->CR2 &= ~I2C_CR2_ITEVTEN;

  i2c_handle->tx_rx_state = I2C_READY;
  i2c_handle->tx_buffer = NULL;
  i2c_handle->tx_length = 0;
}

static void I2C_CloseReceiveData(i2c_handle_t *i2c_handle)
{
  // disable ITBUFEN control bit
  i2c_handle->i2cx->CR2 &= ~I2C_CR2_ITBUFEN;

  // disable ITEVTEN control bit
  i2c_handle->i2cx->CR2 &= ~I2C_CR2_ITEVTEN;

  i2c_handle->tx_rx_state = I2C_READY;
  i2c_handle->rx_buffer = NULL;
  i2c_handle->rx_length = 0;
  i2c_handle->rx_size = 0;

  // Check with `If statement` if you have provided User configuration option for ACK enable or disable
  i2c_handle->i2cx->CR1 |= I2C_CR1_ACK;
}

