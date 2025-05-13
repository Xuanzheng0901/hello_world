#include "OLED.h"

static i2c_master_bus_handle_t bus_handle = NULL;
esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_panel_handle_t panel_handle = NULL;

static void OLED_I2CHW_Init()
{
	i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_1,
        .sda_io_num = OLED_SDA,
        .scl_io_num = OLED_SCL,
        .flags.enable_internal_pullup = true,
    };
	i2c_new_master_bus(&bus_config, &bus_handle);

	esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = 0x3C,
        .scl_speed_hz = 700 * 1000,
        .control_phase_bytes = 1,               // According to SSD1306 datasheet
        .lcd_cmd_bits = 8,   // According to SSD1306 datasheet
        .lcd_param_bits = 8, // According to SSD1306 datasheet
        .dc_bit_offset = 6,                     // According to SH1107 datasheet
    };
	esp_lcd_new_panel_io_i2c(bus_handle, &io_config, &io_handle);

	esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = 64,
    };
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = -1,
		.vendor_config = &ssd1306_config
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
	esp_lcd_panel_io_tx_param(io_handle, 0xA1, NULL, 0);
	esp_lcd_panel_io_tx_param(io_handle, 0xC8, NULL, 0);
	// esp_lcd_panel_io_tx_param(io_handle, 0x20, (uint8_t[]) {0x10}, 1);
    esp_lcd_panel_io_tx_param(io_handle, 0x20, (uint8_t[]) {0x01}, 1);
    esp_lcd_panel_io_tx_param(io_handle, 0x21, (uint8_t[]) {0x00, 0x7F}, 2);
    esp_lcd_panel_io_tx_param(io_handle, 0x22, (uint8_t[]) {0x00, 0x07}, 2);
}

void OLED_Clear(void)
{  
	for(int i = 0; i < 1024; i++)
		esp_lcd_panel_io_tx_color(io_handle, -1, (uint8_t[]){0x00}, 1);
}

void OLED_SetCusor(uint8_t page, uint8_t column)
{
	esp_lcd_panel_io_tx_param(io_handle, 0xB0 | page, NULL, 0);
	esp_lcd_panel_io_tx_param(io_handle, 0x10 | ((column & 0xF0) >> 4), NULL, 0);
	esp_lcd_panel_io_tx_param(io_handle, 0x00 | (column & 0x0F), NULL, 0);
}

void OLED_Point(uint8_t x, uint8_t data)
{
	static const uint8_t mask_table[8] = {
        0x00, 0x01, 0x03, 0x07,
        0x0F, 0x1F, 0x3F, 0x7F
    };
	static uint8_t buffer[8] = {0};

	for(int8_t i = 0; i < 8; i++)
    {
        buffer[i] = ~((data >= 8) ? 0xFF : mask_table[data]); //data
        data = (data > 8) ? (data - 8) : 0; //data > 8 : data - 8; data <= 8: 0
	}
    esp_lcd_panel_io_tx_color(io_handle, -1, buffer, 8);
	// for(uint8_t i = 0; i < 8; i++)
    // {
    //     OLED_SetCusor(i, x);
    //     esp_lcd_panel_io_tx_color(io_handle, -1, (uint8_t[]){buffer[i]}, 1);
    // }

}

void OLED_Init(void)
{
	vTaskDelay(10);//延时100ms
	OLED_I2CHW_Init();			//端口初始化
	OLED_Clear();
}