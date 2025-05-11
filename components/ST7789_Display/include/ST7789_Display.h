#ifndef __ST7789_DIS_H
#define __ST7789_DIS_H

#define SCREEN_WIDTH          320
#define SCREEN_HEIGHT         240

#define DISP_SPI_NUM          SPI2_HOST

#define DISP_GPIO_SCLK       (GPIO_NUM_18)
#define DISP_GPIO_MOSI       (GPIO_NUM_17)
#define DISP_GPIO_RST        (GPIO_NUM_15)
#define DISP_GPIO_DC         (GPIO_NUM_16)
#define DISP_GPIO_CS         (GPIO_NUM_7)
#define DISP_GPIO_BL         (GPIO_NUM_10)
#define DISP_BL_ON_LEVEL      1

#include "esp_lvgl_port.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_lcd_panel_st7789.h"
#include "lv_demos.h"
#include "lv_examples.h"

void Screen_Init(void);
void canvas_print(uint8_t index, uint32_t data);
//void lv_canvas_demo(void);

#endif