#ifndef __HTTPD_H
#define __HTTPD_H

#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_err.h"
#include "Resistor.h"
#include "Power_Control.h"
#include "esp_netif.h"
#include "HTTP_pwdinfo.h"

void HTTP_Init(void);
void update_dns_record(void);

#endif