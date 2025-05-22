#include "aDc.h"
#include "OLED.h"
#include "freertos/FreeRTOS.h"
#include "Resistor.h"
#include "Power_Control.h"
#include "WIFI.h"
#include "HTTP.h"
#include "temp_sensor.h"

void app_main(void)
{
    WIFI_Init();
    HTTP_Init();
    Resistor_Init();
    Pwr_ctrl_Init();
    OLED_Init();
    ADC_Init();
    //DS18B20_Init();
    //update_dns_record();
}