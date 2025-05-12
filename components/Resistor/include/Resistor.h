#ifndef __RESIS_H
#define __RESIS_H

#include "driver/i2c_master.h"

extern uint8_t wiper_pos;
void set_resis_level(uint8_t data);
void set_resis_level_by_percent(uint8_t percent_data);

#endif