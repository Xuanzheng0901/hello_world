#include "aDc.h"
//#include "Display.h"
#include "freertos/FreeRTOS.h"
#include "ST7789_Display.h"
void app_main(void)
{
    ESP_LOGI("Free heap", "%ld bytes\n", esp_get_free_heap_size());
    Screen_Init();
    ESP_LOGI("Free heap", "%ld bytes\n", esp_get_free_heap_size());
    //lv_canvas_demo();
    //OLED_Init();
    // xTaskCreate(canvas_update, "example_lvgl_demo_ui", 8192, NULL, 5, NULL);
    
    //continuous_adc_init();
    //vTaskDelay(10);
    xTaskCreatePinnedToCore(adc_read_task, "adc_read_task", 16384, NULL, 5, NULL, 1);
    // while (1) {
    //     ESP_LOGI("Free heap", "%ld bytes\n", esp_get_free_heap_size());
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}