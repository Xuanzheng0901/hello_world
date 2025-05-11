#include "aDc.h"
//#include "Display.h"
#include "OLED.h"
#include "freertos/FreeRTOS.h"

void app_main(void)
{
    ESP_LOGI("Free heap", "%ld bytes\n", esp_get_free_heap_size());
    OLED_Init();
    continuous_adc_init();
    xTaskCreatePinnedToCore(adc_read_task, "adc_read_task", 16384, NULL, 5, NULL, 1);

}