#include <stdio.h>
#include "ST7789_Display.h"

#define TAG "Display"

static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static lv_display_t *lvgl_disp = NULL;

static void app_lcd_init(void)
{
    /* LCD initialization */
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = DISP_GPIO_SCLK,
        .mosi_io_num = DISP_GPIO_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t),
    };
    spi_bus_initialize(DISP_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO);

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = DISP_GPIO_DC,
        .cs_gpio_num = DISP_GPIO_CS,
        .pclk_hz = 60000000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)DISP_SPI_NUM, &io_config, &lcd_io);

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = DISP_GPIO_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel);

    esp_lcd_panel_reset(lcd_panel);
    esp_lcd_panel_init(lcd_panel);
    esp_lcd_panel_disp_on_off(lcd_panel, true);

    /* LCD backlight on */
    ESP_ERROR_CHECK(gpio_set_level(DISP_GPIO_BL, DISP_BL_ON_LEVEL));
}

static void app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    lvgl_port_init(&lvgl_cfg);

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = 2048,
        .double_buffer = 1,
        .hres = SCREEN_WIDTH,
        .vres = SCREEN_HEIGHT,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = 1,
            .mirror_x = false,
            .mirror_y = 1,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
        }};
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
}
static lv_color_t canvas_buf[280*220];

static lv_obj_t *canvas = NULL;
void lv_example_canvas_board(void)
{
    canvas = lv_canvas_create(lv_scr_act());
    lv_obj_set_size(canvas, 320, 240);
    lv_canvas_set_buffer(canvas, canvas_buf, 320, 240, LV_COLOR_FORMAT_RGB565);
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
    lv_canvas_fill_bg(canvas, lv_color_hex(0x000000), LV_OPA_COVER);
}

void Screen_Init(void)
{
    app_lcd_init();

    app_lvgl_init();

    lvgl_port_lock(0);
    lv_example_canvas_board();
    // lv_example_menu_2();
    //  lv_example_anim_timeline_1();
    //lv_disp_set_rotation(NULL, LV_DISPLAY_ROTATION_180);
    lvgl_port_unlock();
    // lv_demo_display();
}

void canvas_print(uint8_t index, uint32_t data)
{
    index *= 2;
    if(lvgl_port_lock(0))
    {
        for(int i = 0; i < 256; i++)
        {
            lv_canvas_set_px(
                canvas,
                32 + index,
                i,
                i > data ? lv_color_make(0, 0, 0) : lv_color_make(0, 255, 0),
                LV_OPA_COVER
            );
            lv_canvas_set_px(
                canvas,
                32 + index + 1,
                i,
                i > data ? lv_color_make(0, 0, 0) : lv_color_make(0, 255, 0),
                LV_OPA_COVER
            );
        }
        lvgl_port_unlock();
    }
}

