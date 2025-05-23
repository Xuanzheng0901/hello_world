#ifndef __LED_H
#define __LED_H

#define LED_R 21
#define LED_G 14
#define LED_B 13

#define LED_CHANNEL_R LEDC_CHANNEL_0
#define LED_CHANNEL_G LEDC_CHANNEL_1
#define LED_CHANNEL_B LEDC_CHANNEL_2

void LED_Init(void);

#endif