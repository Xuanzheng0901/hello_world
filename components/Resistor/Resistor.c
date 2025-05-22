#include "Resistor.h"

static spi_device_handle_t resis_spi_handle = NULL;
uint8_t wiper_pos = 0x80;

void set_resis_level(uint8_t data)
{
	uint8_t buf[2] = {0x11, 0xFF - data};
	static spi_transaction_t _;	
	_.length = 16;
	_.tx_buffer = buf;
	spi_device_polling_transmit(resis_spi_handle, &_);
	wiper_pos = data;
}

void set_resis_level_by_percent(uint8_t percent_data)
{
    double percent = percent_data;
    percent = percent / 100.0 * 255.0;
    set_resis_level((uint8_t)percent);
}

void Resistor_Init(void)
{
	spi_bus_config_t cfg = {
		.mosi_io_num = RESIS_SDA,
		.sclk_io_num = RESIS_SCL,
	};
	spi_bus_initialize(SPI2_HOST, &cfg, SPI_DMA_CH_AUTO);
	
	spi_device_interface_config_t dev_config = {
		.clock_source = SPI_CLK_SRC_DEFAULT,
		.clock_speed_hz = 1*1000*1000,
		.mode = 0,
		.spics_io_num = RESIS_CS,
		.cs_ena_pretrans = 1,
		.cs_ena_posttrans = 1,
		.queue_size = 2
	};
	spi_bus_add_device(SPI2_HOST, &dev_config, &resis_spi_handle);
    set_resis_level(0xFF);
}