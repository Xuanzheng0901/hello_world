#ifndef __TEMP_SENSOR_H
#define __TEMP_SENSOR_H

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ds18b20.h"
#include "onewire_bus.h"

void DS18B20_Init(void);

#endif