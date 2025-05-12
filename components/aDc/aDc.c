#include "aDc.h"

#define READ_LEN  4096
#define CONF_ADC_CHANNEL    ADC_CHANNEL_1

adc_continuous_handle_t adc_audio_handle = NULL;
adc_continuous_handle_t adc_current_handle = NULL;

static const char *TAG = "ADC";

void continuous_adc_init(void)
{
    ESP_LOGI(TAG, "ADC Initializing...");
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 4096,
        .conv_frame_size = 4,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_audio_handle));
    adc_digi_pattern_config_t adc_pattern = {
        .atten = ADC_ATTEN_DB_12,
        .channel = CONF_ADC_CHANNEL,
        .unit = ADC_UNIT_1,
        .bit_width = ADC_BITWIDTH_12,
    };

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 700,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
        .pattern_num = 1,
        .adc_pattern = &adc_pattern
    };

    adc_continuous_config(adc_audio_handle, &dig_cfg);
    adc_continuous_start(adc_audio_handle);
    ESP_LOGI(TAG, "ADC Initialized!");
}

void adc_read_task(void *pvParameters)
{
    esp_err_t ret;
    uint8_t convert_result[READ_LEN] = {0};
        
    while(1)
    {
        uint32_t ret_num = 0;
        uint8_t read_count = 0;
        for(uint8_t i = 0; i < 128; i++)
        {
            ret = adc_continuous_read(adc_audio_handle, convert_result, 4, &ret_num, 10);
            if(ret == ESP_OK)
            {
                double temp_sum = 0;
                // //temp_sum = temp_sum / 4095.0 * 3300.0; //电压 单位mV
                adc_digi_output_data_t *p = (adc_digi_output_data_t *)&convert_result[0];
                temp_sum += p->type2.data;
                temp_sum /= 60.0;
                OLED_Point(i, 64-temp_sum);
            }
        }
        if(read_count++ >= 2)
            vTaskDelay(1);
    }
}
