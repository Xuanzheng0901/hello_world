#ifndef __RESIS_H
#define __RESIS_H

#include "driver/i2c_master.h"
//#include "driver/i2c.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define RESIS_SDA 18
#define RESIS_SCL 17

extern uint8_t wiper_pos;

void Resistor_Init(void);
void set_resis_level(uint8_t data);
void set_resis_level_by_percent(uint8_t percent_data);

#endif