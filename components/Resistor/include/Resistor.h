#ifndef __RESIS_H
#define __RESIS_H

#include <stdio.h>
#include "esp_log.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"

#define RESIS_SDA 16
#define RESIS_SCL 15
#define RESIS_CS  12

extern uint8_t wiper_pos;

void Resistor_Init(void);
void set_resis_level(uint8_t data);
void set_resis_level_by_percent(uint8_t percent_data);

#endif