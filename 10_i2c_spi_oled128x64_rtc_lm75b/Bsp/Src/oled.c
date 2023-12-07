#include "oled.h"
#include <string.h>


static void oled_reset(void);
static void oled_gpio_spi_pin_config(void);
static void oled_spi_config(void);
static void oled_write_command(uint8_t command);
static void oled_write_data(uint8_t* data, size_t data_size);
static void oled_set_contrast(const uint8_t value);
static void oled_draw_pixel(uint8_t x, uint8_t y, oled_ssd1306_color_t color);
//static void oled_write(uint8_t value);
static void oled_udelay(uint32_t count);
static void oled_mdelay(uint32_t count);

static oled_ssd1306_t oled;
static uint8_t oled_buffer[OLED_SSD1306_BUFFER_SIZE];

extern oled_font_t font_xsmall;
extern oled_font_t font_small;
extern oled_font_t font_large;
extern oled_font_t font_xlarge;

void test_font(void)
{
  oled_fill(black);
  oled_set_cursor(3, 0);
  oled_write_str("24C", font_xlarge, white);
  oled_set_cursor(2, 44);
  oled_write_str("10:58 pm", font_xsmall, white);
  oled_set_cursor(2, 54);
  oled_write_str("TUE / 05.12.2023", font_xsmall, white);
  oled_update_view();
}

void oled_init(void)
{
  oled_gpio_spi_pin_config();
  oled_spi_config();
  oled_reset();
  oled_mdelay(100);             // 100ms delay page: 27, power on and off sequence
  oled_display(OFF);

  oled_write_command(0x20);     // set Memory Addressing Mode
  oled_write_command(0x00);     // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
                                // 10b,Page Addressing Mode (RESET); 11b,Invalid

  oled_write_command(0xB0);     // set Page Start Address for Page Addressing Mode,0-7

//#ifdef SSD1306_MIRROR_VERT
//  oled_write_command(0xC0);     // mirror vertically
//#else
  oled_write_command(0xC8);     // set COM Output Scan Direction
//#endif

  oled_write_command(0x00);     // set low column address
  oled_write_command(0x10);     // set high column address

  oled_write_command(0x40);     // set start line address

  oled_set_contrast(0xFF);

//#ifdef SSD1306_MIRROR_HORIZ
//  oled_write_command(0xA0);     // mirror horizontally
//#else
  oled_write_command(0xA1);     // set segment re-map 0 to 127
//#endif

//#ifdef SSD1306_INVERSE_COLOR
//  oled_write_command(0xA7);   // set inverse color
//#else
  oled_write_command(0xA6);     // set normal color
//#endif

// Set multiplex ratio.
//#if (SSD1306_HEIGHT == 128)
  // Found in the Luma Python lib for SH1106.
  //oled_write_command(0xFF);
//#else
  oled_write_command(0xA8); // set multiplex ratio(1 to 64)
//#endif
//
//#if (SSD1306_HEIGHT == 32)
//  oled_write_command(0x1F);
//#elif (SSD1306_HEIGHT == 64)
  oled_write_command(0x3F);
//#elif (SSD1306_HEIGHT == 128)
//  oled_write_command(0x3F);
//#else
//#error "Only 32, 64, or 128 lines of height are supported!"
//#endif

  oled_write_command(0xA4);    // 0xa4, output follows RAM content;0xa5,Output ignores RAM content

  oled_write_command(0xD3);    // set display offset - CHECK
  oled_write_command(0x00);    // no offset

  oled_write_command(0xD5);    // set display clock divide ratio/oscillator frequency
  oled_write_command(0xF0);    // set divide ratio

  oled_write_command(0xD9);    // set pre-charge period
  oled_write_command(0x22);

  oled_write_command(0xDA);    // set com pins hardware configuration
//#if (SSD1306_HEIGHT == 32)
//  oled_write_command(0x02);
//#elif (SSD1306_HEIGHT == 64)
  oled_write_command(0x12);
//#elif (SSD1306_HEIGHT == 128)
//  oled_write_command(0x12);
//#else
//#error "Only 32, 64, or 128 lines of height are supported!"
//#endif

  oled_write_command(0xDB);    // set vcomh
  oled_write_command(0x20);    //0x20,0.77xVcc

  oled_write_command(0x8D);    // set DC-DC enable
  oled_write_command(0x14);    //
  oled_display(ON);

  // clear screen
  oled_fill(black);

  // flush buffer to screen
  oled_update_view();

  // set default values for screen object
  oled.current_x = 0;
  oled.current_y = 0;

  oled.initialised = 1;
}

static void oled_reset(void)
{
//  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_SPI_NSS_GPIO_PIN, GPIO_PIN_SET); // deselect CS pin
  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_RESET_PIN, GPIO_PIN_RESET);      // low means initialisation of chip page: 1, figure 7-1
  oled_udelay(10);    // (t1) 3us delay page: 27, 8.9 power on and off sequence
  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_RESET_PIN, GPIO_PIN_SET);
  oled_udelay(10);    // (t2) 3us delay page: 27, figure 8-16
}

static void oled_write_command(uint8_t command)
{
//  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_SPI_NSS_GPIO_PIN, GPIO_PIN_RESET); // Pull CS low (select)
  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_DATA_COMMAND_GPIO_PIN, GPIO_PIN_RESET); // pull DC low for command, page: 14
  SPI_SendData(OLED_SPI, &command, 1);
//  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_SPI_NSS_GPIO_PIN, GPIO_PIN_SET);   // Pull CS high (unselect)
}

static void oled_write_data(uint8_t* data, size_t data_size)
{
//  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_SPI_NSS_GPIO_PIN, GPIO_PIN_RESET); // Pull CS low (select)
  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_DATA_COMMAND_GPIO_PIN, GPIO_PIN_SET); // pull DC high for data, page: 14
  SPI_SendData(OLED_SPI, data, data_size);
//  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_SPI_NSS_GPIO_PIN, GPIO_PIN_SET);   // Pull CS high (unselect)
}

char oled_write_char(char ch, oled_font_t font, oled_ssd1306_color_t color)
{
  uint32_t i, b, j;

  // Check if character is valid
  if (ch < 32 || ch > 126)
	return 0;

  // Check remaining space on current line
  if (OLED_SSD1306_WIDTH < (oled.current_x + font.font_width) ||
	  OLED_SSD1306_HEIGHT < (oled.current_y + font.font_height))
  {
	// Not enough space on current line
	return 0;
  }

  // Use the font to write
  for(i = 0; i < font.font_height; i++)
  {
	b = font.data[(ch - 32) * font.font_height + i];
	for(j = 0; j < font.font_width; j++)
	{
	  if((b << j) & 0x8000)
	  {
		oled_draw_pixel(oled.current_x + j, (oled.current_y + i), (oled_ssd1306_color_t) color);
	  }
	  else
	  {
		oled_draw_pixel(oled.current_y + j, (oled.current_y + i), (oled_ssd1306_color_t)!color);
	  }
	}
  }

  // The current space is now taken
  oled.current_x += font.font_width;

  // Return written char for validation
  return ch;
}


static void oled_draw_pixel(uint8_t x, uint8_t y, oled_ssd1306_color_t color)
{
  if(x >= OLED_SSD1306_WIDTH || y >= OLED_SSD1306_HEIGHT)
  {
	// Don't write outside the buffer
	return;
  }

  // Draw in the right color
  if(color == white)
  {
	oled_buffer[x + (y / 8) * OLED_SSD1306_WIDTH] |= 1 << (y % 8);
  }
  else
  {
	oled_buffer[x + (y / 8) * OLED_SSD1306_WIDTH] &= ~(1 << (y % 8));
  }
}

char oled_write_str(char* str, oled_font_t font, oled_ssd1306_color_t color)
{
  while (*str)
  {
	if (oled_write_char(*str, font, color) != *str)
	{
	  // char could not be written
	  return *str;
	}
	str++;
  }
  // works!!
  return *str;
}

void oled_set_cursor(uint8_t x, uint8_t y)
{
  oled.current_x = x;
  oled.current_y = y;
}

static void oled_gpio_spi_pin_config(void)
{
  gpio_handle_t spi_pins;

  memset(&spi_pins,0,sizeof(gpio_handle_t));

  spi_pins.gpiox                   = OLED_SPI_GPIO_PORT;
  spi_pins.gpio_config.pin_mode    = OLED_SPI_GPIO_PIN_MODE;
  spi_pins.gpio_config.pin_alt_fun = OLED_SPI_GPIO_ALT_FUN;
  spi_pins.gpio_config.pin_op_type = OLED_SPI_GPIO_OPTYPE;
  spi_pins.gpio_config.pin_pu_pd   = OLED_SPI_GPIO_PUPD;
  spi_pins.gpio_config.pin_speed   = OLED_SPI_GPIO_PIN_SPEED;

  // NSS
  spi_pins.gpio_config.pin_number  = OLED_SPI_NSS_GPIO_PIN;
  GPIO_Init(&spi_pins);

  // SCK
  spi_pins.gpio_config.pin_number  = OLED_SPI_SCK_GPIO_PIN;
  GPIO_Init(&spi_pins);

  // MOSI
  spi_pins.gpio_config.pin_number  = OLED_SPI_MOSI_GPIO_PIN;
  GPIO_Init(&spi_pins);

  // DC (Data/Command) pin OLED SSD1306
  spi_pins.gpio_config.pin_mode    = GPIO_MODE_OUT;
  spi_pins.gpio_config.pin_number  = OLED_DATA_COMMAND_GPIO_PIN;
  GPIO_Init(&spi_pins);

  // RES (RESET) pin OLED SSD1306
  spi_pins.gpio_config.pin_number  = OLED_RESET_PIN;
  GPIO_Init(&spi_pins);
}

static void oled_spi_config(void)
{
  spi_handle_t spi2_handle;

  spi2_handle.spix                        = OLED_SPI;
  spi2_handle.spi_config.spi_bus_config   = OLED_SPI_BUS_CONFIG;
  spi2_handle.spi_config.spi_device_mode  = OLED_SPI_DEVICE_MODE;
  spi2_handle.spi_config.spi_sck_speed    = OLED_SPI_SCLK_SPEED;
  spi2_handle.spi_config.spi_dff          = OLED_SPI_DFF;
  spi2_handle.spi_config.spi_cpha         = OLED_SPI_CPHA;
  spi2_handle.spi_config.spi_cpol         = OLED_SPI_CPOL;
  spi2_handle.spi_config.spi_ssm          = OLED_SPI_SSM;
  SPI_Init(&spi2_handle);
}

void oled_update_view(void)
{
  for (uint8_t i=0; i<OLED_SSD1306_HEIGHT/8; i++)
  {
	oled_write_command(0xB0+i); // set current RAM page address
	oled_write_command(0x00+OLED_X_OFFSET_LOWER);
	oled_write_command(0x10+OLED_X_OFFSET_UPPER);
	oled_write_data(&oled_buffer[OLED_SSD1306_WIDTH*i], OLED_SSD1306_WIDTH);
  }
}

void oled_fill(oled_ssd1306_color_t color)
{
  memset(oled_buffer, (color==black) ? 0x00:0xFF, sizeof(oled_buffer));
}

static void oled_set_contrast(const uint8_t value)
{
  const uint8_t contrast_control_register = 0x81;
  oled_write_command(contrast_control_register);
  oled_write_command(value);
}

//static void oled_write(uint8_t value)
//{
//  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_SPI_NSS_GPIO_PIN, GPIO_PIN_RESET); // Pull CS low (select)
//  SPI_SendData(OLED_SPI, &value, 1);
//  GPIO_WriteToOutputPin(OLED_SPI_GPIO_PORT, OLED_SPI_NSS_GPIO_PIN, GPIO_PIN_SET);   // Pull CS high (unselect)
//}

void oled_display(uint8_t on)
{
  uint8_t value;
  if (on)
  {
	value = 0xAF;
	oled.display_on = ON;
  }
  else
  {
	value = 0xAE;
	oled.display_on = OFF;
  }
  oled_write_command(value);
}

static void oled_mdelay(uint32_t count)
{
  for (uint32_t i=0; i< (count * 1000); i++);
}

static void oled_udelay(uint32_t count)
{
  for (uint32_t i=0; i< (count * 1); i++);
}
