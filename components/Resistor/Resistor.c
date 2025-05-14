#include "Resistor.h"

// static i2c_master_bus_handle_t bus_handle;
// static i2c_master_dev_handle_t dev_handle;

// void set_resis_level(uint8_t data)
// {
//     uint8_t buf[2] = {0x01, data};
//     ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, buf, sizeof(buf), 10));
//     // i2c_master_transmit(dev_handle, (uint8_t[]){0x00, data}, 5, 10);
//     // i2c_master_transmit(dev_handle, (uint8_t[]){0x00, data}, 5, 10);
// }

// void set_resis_level_by_percent(uint8_t percent_data)
// {
//     double percent = percent_data;
//     percent /= 128.0;
//     set_resis_level((uint8_t)percent);
// }

// void Resistor_Init(void)
// {
//     i2c_master_bus_config_t bus_config = {
//         .clk_source = I2C_CLK_SRC_DEFAULT,
//         .glitch_ignore_cnt = 7,
//         .i2c_port = I2C_NUM_0,
//         .sda_io_num = RESIS_SDA,
//         .scl_io_num = RESIS_SCL,
//         .flags.enable_internal_pullup = true,
//         .trans_queue_depth = 8,
//     };
    
//     i2c_new_master_bus(&bus_config, &bus_handle);

//     i2c_device_config_t dev_config = {
//         .dev_addr_length = I2C_ADDR_BIT_LEN_7,
//         .device_address = 0x3E,
//         .flags.disable_ack_check = true,
//         .scl_speed_hz = 100 * 1000,
//     };
//     i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle);
//     // for(uint16_t i = 0; i < 0x7F; i++)
//     // {
//     //     ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_probe(bus_handle, i, 10));
//     //     vTaskDelay(10);
//     // }
//     // for(int i = 0; i < 0xFF; i++)
//     // {
//         //i2c_master_probe(bus_handle, 85, 10);
//     //     vTaskDelay(50);
//     // }
    
//     //ESP_LOGI("RESIS", "dev addr: %d, dev addr length: %d ");
//     set_resis_level(0x00);
// }

static void OLED_W_SCL(uint8_t a)
{
	gpio_set_level(RESIS_SCL, a);
    esp_rom_delay_us(2);
}

static void OLED_W_SDA(uint8_t a)
{
    
	gpio_set_level(RESIS_SDA, a);
    esp_rom_delay_us(2);
}


static void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

static void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

static void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

void set_resis_level(uint8_t data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x7C);		//从机地址
	OLED_I2C_SendByte(0x00);		//写数据
	OLED_I2C_SendByte(data);
	OLED_I2C_Stop();
}

void Resistor_Init(void)
{
	gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = 1ULL<<RESIS_SCL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = 1ULL<<RESIS_SDA;
	gpio_config(&io_conf);
	OLED_W_SCL(1);
	OLED_W_SDA(1);

    set_resis_level(0x7F);
}