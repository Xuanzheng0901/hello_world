#ifndef __OLED_H
#define __OLED_H
#include "stdio.h"
#include <string.h>
#include "driver/gpio.h"
#include "stdio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define SCL 18
#define SDA 8

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Point(uint8_t x, uint8_t data);

#endif
