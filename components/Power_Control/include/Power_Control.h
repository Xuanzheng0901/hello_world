#ifndef __PWR_CTRL_H
#define __PWR_CTRL_H

#define P_SWI 42
#define N_SWI 41

#include "stdio.h"
#include "driver/gpio.h"
#include "esp_log.h"

void Pwr_ctrl_Init(void);
void Pwr_ctrl_P(bool level);
void Pwr_ctrl_N(bool level);

#endif