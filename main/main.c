#include "aDc.h"
#include "OLED.h"
#include "freertos/FreeRTOS.h"
#include "Resistor.h"
#include "Power_Control.h"
#include "WIFI.h"
#include "HTTP.h"
#include "temp_sensor.h"
#include "LED.h"

void app_main(void)
{
    wifi_init();
    HTTP_Init();
    
    LED_Init();
    Pwr_ctrl_Init();
    Resistor_Init();
    OLED_Init();
    ADC_Init();
    //DS18B20_Init();
    
    //update_dns_record();
}