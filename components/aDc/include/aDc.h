#ifndef __ADC_H
#define __ADC_H

#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_continuous.h"
#include "math.h"
#include "OLED.h"

void continuous_adc_init(void);
void adc_read_task(void *pvParameters);


#endif