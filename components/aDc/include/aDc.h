#ifndef __ADC_H
#define __ADC_H

#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "esp_adc/adc_continuous.h"
#include "math.h"
#include "OLED.h"
#include "Power_Control.h"
#include "driver/gpio.h"

#define READ_LEN  4096
#define AUDIO_CHANNEL         ADC_CHANNEL_1
#define CURR_DETC_CHANNEL_P   ADC_CHANNEL_9
#define CURR_DETC_CHANNEL_N   ADC_CHANNEL_8

void ADC_Init(void);

#endif