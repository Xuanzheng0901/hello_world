#include <stdio.h>
#include "LED.h"
#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"
#include "esp_log.h"

extern uint8_t warning_count;
uint8_t rgb[3] = {255, 0, 0};
int current = 1;
int previous = 0;

TaskHandle_t LED_Blink_handle = NULL;

void led_set(uint8_t index, uint8_t value)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, index, value); 
    ledc_update_duty(LEDC_LOW_SPEED_MODE, index);
}

void LED_Blink(void *arg)
{
    while (1)
    {
        // 上升通道 +1
        if (rgb[current] < 255) {
            rgb[current]++;
            led_set(current, rgb[current]);
        }

        // 下降通道 -1
        if (rgb[previous] > 0) {
            rgb[previous]--;
            led_set(previous, rgb[previous]);
        }

        // 一段颜色渐变完成，切换到下一对通道
        if (rgb[current] == 255 && rgb[previous] == 0) {
            previous = current;
            current = (current + 1) % 3;
        }

        vTaskDelay(1); // 控制渐变速度
    }
}

void LED_Warning(void *arg)
{
    uint8_t value = 255;
    while(1)
    {
        if(warning_count)
        {
            ESP_LOGI("LED", "Warning Detect!");
            vTaskDelete(LED_Blink_handle);
            led_set(LED_CHANNEL_G, 0);
            led_set(LED_CHANNEL_B, 0);
            led_set(LED_CHANNEL_R, 0);
            while(warning_count)
            {
                value = value ? 0 : 255;
                vTaskDelay(100 / warning_count);
                for(uint8_t i = 0; i < 3; i++)
                {
                    if(1 << i & warning_count)
                        led_set(i, value);
                }
            }
            for(int i = 0; i < 3; i++)
                led_set(i, rgb[i]);
            xTaskCreate(LED_Blink, "LED流光", 4096, NULL, 5, &LED_Blink_handle);
        }
        vTaskDelay(2);
    }
}

void LED_Init(void)
{
    ledc_timer_config_t timer_config = {
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = LEDC_TIMER_9_BIT,
        .freq_hz = 1024,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&timer_config);

    ledc_channel_config_t channel_config = {
        .channel = LED_CHANNEL_R,
        .duty = 0, 
        .flags.output_invert = 0,
        .gpio_num = LED_R,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&channel_config);

    channel_config.channel = LED_CHANNEL_G;
    channel_config.gpio_num = LED_G,
    ledc_channel_config(&channel_config);

    channel_config.channel = LED_CHANNEL_B;
    channel_config.gpio_num = LED_B,
    ledc_channel_config(&channel_config);
    //ledc_fade_func_install();

    xTaskCreate(LED_Blink, "LED流光", 4096, NULL, 5, &LED_Blink_handle);
    xTaskCreate(LED_Warning, "LED警报", 4096, NULL, 5, NULL);
}