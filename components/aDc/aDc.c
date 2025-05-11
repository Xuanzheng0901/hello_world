#include "aDc.h"

#define READ_LEN  4096
#define CONF_ADC_CHANNEL    ADC_CHANNEL_4

adc_continuous_handle_t handle = NULL;
static const char *TAG = "ADC";

void continuous_adc_init(void)
{
    ESP_LOGI(TAG, "ADC Initializing...");
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 4096,
        .conv_frame_size = READ_LEN,
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));
    adc_digi_pattern_config_t adc_pattern = {
        .atten = ADC_ATTEN_DB_12,
        .channel = CONF_ADC_CHANNEL,
        .unit = ADC_UNIT_1,
        .bit_width = ADC_BITWIDTH_12,
    };

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 10*1024,
        // 需要128个平均值数据(每位一个) 取8个数据的平均值 每帧显示1024个数据 帧率20 每秒20480个数据 转换速度20k
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
        .pattern_num = 1,
        .adc_pattern = &adc_pattern
    };

    adc_continuous_config(handle, &dig_cfg);
    adc_continuous_start(handle);
    ESP_LOGI(TAG, "ADC Initialized!");
}

void adc_read_task(void *pvParameters)
{
    continuous_adc_init();
    //vTaskDelay(10);
    esp_err_t ret;
    uint8_t convert_result[READ_LEN] = {0};
    //uint32_t data_array[1024];
    //uint32_t trans_buffer[128];
        
    while(1)
    {
        //vTaskDelay(50);
        uint32_t ret_num = 0;
        //static char str[128];
        /** ret_num: 转换到buffer中的字节数 
        SOC_ADC_DIGI_RESULT_BYTES每个结果的字节数4字节32位(uint32t指针)
        ret_num / SOC_... = buffer中结果的个数**/
        ret = adc_continuous_read(handle, convert_result, READ_LEN, &ret_num, 10); //不要脱裤子放屁
        if(ret == ESP_OK)
        {
            //以下都是求平均值
            //ESP_LOGI(TAG, "ret_num:%ld", ret_num);
            for(int i = 0; i < ret_num / 2; i += 8)
            {
                double temp_sum = 0;
                for(int j = i; j < i + 8; j += 4)
                {
                    adc_digi_output_data_t *p = (adc_digi_output_data_t *)&convert_result[j];
                    temp_sum += p->type2.data;
                }
                temp_sum /= 2.0;
                //temp_sum = -3.12e-6 * temp_sum * temp_sum + 0.945 * temp_sum + 160.0;
                temp_sum = temp_sum / 4095.0 * 3300.0; //电压 单位mV
                temp_sum /= 15.0;
                canvas_print(i / 8, (uint32_t)temp_sum);

                // memset(str, 0, 128 * sizeof(char));
                // temp_sum = temp_sum * 20 / 50;
                // for(int i = 0; i < temp_sum; i++)
                //     strcat(str, "*");
                // ESP_LOGI(TAG, "%s" ,str);
            }
        }
    }
}
