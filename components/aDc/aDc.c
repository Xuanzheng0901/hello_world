#include "aDc.h"

#define READ_LEN  4096
#define AUDIO_CHANNEL         ADC_CHANNEL_1
#define CURR_DETC_CHANNEL_P   ADC_CHANNEL_9
#define CURR_DETC_CHANNEL_N   ADC_CHANNEL_8

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
    adc_continuous_config(adc_audio_handle, &dig_cfg);
    adc_continuous_start(adc_audio_handle);
    ESP_LOGI(TAG, "ADC Initialized!");
}

void adc_read_task(void *pvParameters)
{
    esp_err_t ret;
    uint8_t read_count = 0;
    uint8_t convert_result[READ_LEN] = {0};
    static adc_digi_output_data_t *read_data[8];
    
    while(1)
    {
        uint32_t ret_num = 0;
        for(uint8_t i = 0; i < 12; i++)
        {
            ret = adc_continuous_read(adc_audio_handle, convert_result, 32, &ret_num, 10);
            if(ret != ESP_OK)
                continue;
            for(int result_index = 0, data_index = 0; result_index < ret_num; result_index += 4, data_index++)
                read_data[data_index] = (adc_digi_output_data_t *)&convert_result[result_index];

            uint8_t audio_count = 0;
            double temp_audio_sum = 0;

            for(int j = 0; j < 8; j++)
            {
                switch(read_data[j]->type2.channel)
                {
                    case AUDIO_CHANNEL:
                        temp_audio_sum += read_data[j]->type2.data;
                        audio_count++;
                        break;
                    case CURR_DETC_CHANNEL_N:
                        break;
                    case CURR_DETC_CHANNEL_P:
                        break;
                }
            }

            temp_audio_sum /= (60.0 * (double)audio_count);
            // //temp_sum = temp_sum / 4095.0 * 3300.0; //电压 单位mV
            OLED_Point(i, 64-temp_audio_sum);
        }
        //if(read_count++ >= 2)
            vTaskDelay(1);
    }
}
