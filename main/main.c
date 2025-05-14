#include "aDc.h"
#include "OLED.h"
#include "freertos/FreeRTOS.h"
#include "Resistor.h"
#include "Power_Control.h"
#include "WIFI.h"
#include "HTTP.h"

void app_main(void)
{
    wifi_init();
    HTTP_Init();
    Resistor_Init();
    Pwr_ctrl_Init();
    OLED_Init();
    ADC_Init();
    xTaskCreatePinnedToCore(adc_read_task, "adc_read_task", 16384, NULL, 5, NULL, 1);
}