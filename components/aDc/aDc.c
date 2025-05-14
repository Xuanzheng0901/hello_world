#include "aDc.h"

adc_continuous_handle_t adc_audio_handle = NULL;
adc_continuous_handle_t adc_current_handle = NULL;

static const char *TAG = "ADC";

void ADC_Init(void)
{
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
    ESP_LOGI(TAG, "连续转换ADC初始化完成");
}

void adc_read_task(void *pvParameters)
{
    esp_err_t ret;
    //uint8_t read_count = 0;
    uint8_t convert_result[READ_LEN] = {0};
    static adc_digi_output_data_t *read_data[8];
    uint16_t curr_n_count = 0;
    double temp_curr_sum = 0;
    while(1)
    {
        uint32_t ret_num = 0;
        
        for(uint8_t i = 0; i < 12; i++)
        {
            uint8_t audio_count = 0;
            double temp_audio_sum = 0;

            ret = adc_continuous_read(adc_audio_handle, convert_result, 32, &ret_num, 10);
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
                        temp_curr_sum += read_data[j]->type2.data;
                        curr_n_count++;
                        if(curr_n_count == 256)
                        {
                            temp_curr_sum /= 256.0;
                            //ESP_LOGI("CURR_N", "Current current: %ld", (uint32_t)temp_curr_sum);
                            if((uint32_t)temp_curr_sum >= 3800)
                            {
                                Pwr_ctrl_N(0);
                            }
                            temp_curr_sum = 0.0;
                            curr_n_count = 0;
                        }
                            
                        break;
                    case CURR_DETC_CHANNEL_P:
                        break;
                }
            }

            temp_audio_sum /= (60.0 * (double)audio_count);
            //temp_sum = temp_sum / 4095.0 * 3300.0; //电压 单位mV
            OLED_Point(0, 64-temp_audio_sum);
        }
        //if(read_count++ >= 2)
        vTaskDelay(1);
    }
}
