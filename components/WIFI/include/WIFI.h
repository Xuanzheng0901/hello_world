#ifndef __WIFI_H
#define __WIFI_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <esp_mac.h>
#include "HTTP.h"
#include "WIFI_pwdinfo.h"

void WIFI_Init(void);
esp_err_t wifi_init(void);

#endif