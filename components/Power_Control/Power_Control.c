#include "Power_Control.h"

static const char* TAG = "POWER_CONTROL";
bool power_status[2];

/*
*   @brief +18V电源开关,拉高断开,拉低闭合
*/
void Pwr_ctrl_P(bool level) //+18V
{
    gpio_set_level_mod(P_SWI, level);
    power_status[POWER_P] = level;
    ESP_LOGI(TAG, "+18V电源已%s", level ?  "导通": "断开");
}

/*
    @brief -18V电源开关,拉高断开,拉低闭合
*/
void Pwr_ctrl_N(bool level) //-18V
{
    gpio_set_level_mod(N_SWI, level);
    power_status[POWER_N] = level;
    ESP_LOGI(TAG, "-18V电源已%s", level ? "导通" : "断开");
}

void Pwr_ctrl_Init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 1,
        .pin_bit_mask = 1ULL<<P_SWI
    };
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = 1ULL<<N_SWI;
    gpio_config(&io_conf);
    Pwr_ctrl_P(1);
    Pwr_ctrl_N(1);
    ESP_LOGI(TAG, "电源控制初始化完成");
}