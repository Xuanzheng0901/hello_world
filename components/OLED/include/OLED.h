#ifndef __OLED_H
#define __OLED_H

#include "stdio.h"
#include <string.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_lcd_panel_ssd1306.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#define OLED_SCL 6
#define OLED_SDA 5

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Point(uint8_t x, uint8_t data);

#endif
