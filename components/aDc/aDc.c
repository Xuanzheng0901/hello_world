#include "aDc.h"

#define WARNING_VOLTAGE_H   3200
#define WARNING_VOLTAGE_L   2600

static const char *TAG = "ADC";
static adc_continuous_handle_t adc_handle = NULL;
static uint32_t env_volume = 1;

extern bool power_status[2];

bool auto_control_volume = 1;

static uint8_t opt;
uint8_t warning_count = 0;

void auto_ctrl_switch(void* arg)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0,
        .pull_up_en = 1,
        .pin_bit_mask = 1ULL << 39
    };
    gpio_config(&io_conf);
    while(1)
    {
        if(gpio_get_level(39) == 0)
        {
            while(gpio_get_level(39) == 0)
            {
                vTaskDelay(1);
            }
            auto_control_volume = !auto_control_volume;
        }
        vTaskDelay(1);
    }
}

void Pwr_Stop(void* arg)
{
    uint8_t opt = *(uint8_t*)arg;
    vTaskDelay(opt+1);
    warning_count |= (((uint8_t)1) << opt);
    ESP_LOGI("Warning_count", "%d", warning_count);
    opt ? Pwr_ctrl_N(0) : Pwr_ctrl_P(0);

    while(gpio_get_level(40) && power_status[opt] == 0)
        vTaskDelay(10);

    opt ? Pwr_ctrl_N(1) : Pwr_ctrl_P(1);
    vTaskDelay(opt+1);
    warning_count &= ~(((uint8_t)1) << opt);
    vTaskDelete(NULL);
}


uint32_t get_env_volume(void)
{
    esp_err_t ret;
    uint32_t ret_num = 0, result = 0;
    uint16_t read_count = 0;
    uint8_t convert_result[READ_LEN] = {0};
    static adc_digi_output_data_t *read_data[8];
    ESP_LOGI(TAG, "开始采集环境音量!");
    while(read_count <= 4800)
    {
        for(int i = 0; i < 5; i++)
        {

            ret = adc_continuous_read(adc_handle, convert_result, 32, &ret_num, 10);
            if(ret != ESP_OK)
                continue;
            for(int result_index = 0, data_index = 0; result_index < ret_num; result_index += 4, data_index++)
                read_data[data_index] = (adc_digi_output_data_t *)&convert_result[result_index];
            for(int j = 0; j < 8; j++)
            {
                if(read_data[j]->type2.channel == AUDIO_CHANNEL)
                {
                    result += read_data[j]->type2.data;
                    read_count++;
                }
            }
        }
        ESP_LOGI(TAG, "采集进度: %d%%", read_count / 48);
        vTaskDelay(1);
    }
    double result_f = (double)result;
    result_f /= 4800.0;
    result = (uint32_t) result_f;
    return result;
}


void adc_read_task(void *pvParameters)
{
    env_volume = get_env_volume();
    esp_err_t ret;
    uint8_t convert_result[READ_LEN] = {0};
    static adc_digi_output_data_t *read_data[8];
    uint16_t audio_statistic = 0, noice_count = 0; // 音频数据数量 超过阈值的噪声的数量。
    uint16_t curr_n_count = 0, curr_p_count = 0;
    double temp_curr_sum_p = 0, temp_curr_sum_n = 0;
    while(1)
    {
        uint32_t ret_num = 0;

        for(uint8_t i = 0; i < 12; i++)
        {
            uint8_t audio_count = 0;
            double temp_audio_sum = 0;

            ret = adc_continuous_read(adc_handle, convert_result, 32, &ret_num, 10);
            if(ret != ESP_OK)
                continue;
            for(int result_index = 0, data_index = 0; result_index < ret_num; result_index += 4, data_index++)
                read_data[data_index] = (adc_digi_output_data_t *)&convert_result[result_index];

            for(int j = 0; j < 8; j++)
            {
                switch(read_data[j]->type2.channel)
                {
                    case AUDIO_CHANNEL:
                        temp_audio_sum += read_data[j]->type2.data;
                        if(read_data[j]->type2.data >= env_volume + 500 || read_data[j]->type2.data <= env_volume - 500)
                        {
                            noice_count++;
                        }
                        audio_statistic++;
                        if(audio_statistic == 3600)
                        {
                            double temp = (double)noice_count;
                            temp /= (double)(3600 / 200);
                            if(auto_control_volume)
                            {
                                noice_count = (uint8_t)temp;
                                if(noice_count > 200)
                                    noice_count = 200;
                                //uint8_t new_resis = (wiper_pos + 55 + noice_count) / 2;
                                set_resis_level(200 - noice_count);
                                //ESP_LOGI(TAG, "noice_count: %d", noice_count);
                            }
                            noice_count = 0;
                            audio_statistic = 0;
                        }
                        audio_count++;
                        break;
                    case CURR_DETC_CHANNEL_N:
                        temp_curr_sum_n += read_data[j]->type2.data;
                        curr_n_count++;
                        if(curr_n_count == 128)
                        {
                            temp_curr_sum_n /= 128.0;
                            //ESP_LOGI("CURR_N", "Current current: %ld", (uint32_t)temp_curr_sum_n);
                            if((uint32_t)temp_curr_sum_n >= 3350 && power_status[POWER_N])
                            {
                                ESP_LOGI(TAG, "P:%ld", (uint32_t)temp_curr_sum_n);
                                opt = 1;
                                xTaskCreate(Pwr_Stop, "关闭电源", 4096, &opt, 5, NULL);
                            }
                            temp_curr_sum_n = 0.0;
                            curr_n_count = 0;
                        }
                        break;
                    case CURR_DETC_CHANNEL_P:
                        temp_curr_sum_p += read_data[j]->type2.data;
                        curr_p_count++;
                        if(curr_p_count == 128)
                        {
                            temp_curr_sum_p /= 128.0;
                            //ESP_LOGI("CURR_P", "Current current: %ld", (uint32_t)temp_curr_sum_p);
                            if((uint32_t)temp_curr_sum_p >= 3800 && power_status[POWER_P])
                            {
                                ESP_LOGI(TAG, "N:%ld", (uint32_t)temp_curr_sum_p);
                                opt = 0;
                                xTaskCreate(Pwr_Stop, "关闭电源", 4096, &opt, 5, NULL);
                            }
                            temp_curr_sum_p = 0.0;
                            curr_p_count = 0;
                        }
                        break;
                }
            }
            //ESP_LOGI(TAG, "audio_count: %d", audio_count);
            temp_audio_sum /= (64.0 * (double)audio_count);
            //temp_sum = temp_sum / 4095.0 * 3300.0; //电压 单位mV
            OLED_Point(0, 64-temp_audio_sum);
        }
        vTaskDelay(1);
    }
}

void ADC_Init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0,
        .pull_up_en = 1,
        .pin_bit_mask = 1ULL<<40
    };
    gpio_config(&io_conf);
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 4096,
        .conv_frame_size = 4,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));
    adc_digi_pattern_config_t adc_pattern[3] = {
        {
            .atten = ADC_ATTEN_DB_12,
            .channel = AUDIO_CHANNEL,
            .unit = ADC_UNIT_1,
            .bit_width = ADC_BITWIDTH_12,
        },
        {
            .atten = ADC_ATTEN_DB_12,
            .channel = CURR_DETC_CHANNEL_P,
            .unit = ADC_UNIT_1,
            .bit_width = ADC_BITWIDTH_12,
        },
        {
            .atten = ADC_ATTEN_DB_12,
            .channel = CURR_DETC_CHANNEL_N,
            .unit = ADC_UNIT_1,
            .bit_width = ADC_BITWIDTH_12,
        }
    };

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20*1000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
        .pattern_num = 3,
        .adc_pattern = adc_pattern
    };
    adc_continuous_config(adc_handle, &dig_cfg);
    adc_continuous_start(adc_handle);
    ESP_LOGI(TAG, "连续转换ADC初始化完成");
    xTaskCreatePinnedToCore(adc_read_task, "adc_read_task", 16384, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(auto_ctrl_switch, "自动控制开关", 4096, NULL, 5, NULL, 1);
}