#include "aDc.h"

#define WARNING_VOLTAGE_H   3200
#define WARNING_VOLTAGE_L   2600

static adc_continuous_handle_t adc_handle = NULL;
extern bool power_status[2];
static const char *TAG = "ADC";
static uint8_t opt;

void Pwr_Stop(void* arg)
{
    uint8_t opt = *(uint8_t*)arg;
    gpio_set_level(21, 1);

    opt ? Pwr_ctrl_N(0) : Pwr_ctrl_P(0);

    while(gpio_get_level(40) && power_status[opt] == 0)
        vTaskDelay(10);
            

    opt ? Pwr_ctrl_N(1) : Pwr_ctrl_P(1);
    gpio_set_level(21, 0);
    vTaskDelete(NULL);
}



void adc_read_task(void *pvParameters)
{
    ADC_Init();
    esp_err_t ret;
    
    uint8_t convert_result[READ_LEN] = {0};
    static adc_digi_output_data_t *read_data[8];
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
                        audio_count++;
                        break;
                    case CURR_DETC_CHANNEL_N:
                        temp_curr_sum_n += read_data[j]->type2.data;
                        curr_n_count++;
                        if(curr_n_count == 128)
                        {
                            temp_curr_sum_n /= 128.0;
                            ESP_LOGI("CURR_N", "Current current: %ld", (uint32_t)temp_curr_sum_n);
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
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 1;
    io_conf.pull_down_en = 0;
    io_conf.pin_bit_mask = 1ULL << 21;
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
}