#include "temp_sensor.h"

#define ONEWIRE_BUS_GPIO    11

static int s_ds18b20_device_num = 0;
static float s_temperature = 0.0;
static ds18b20_device_handle_t s_ds18b20s[1];

static const char *TAG = "DS18B20";

static void sensor_detect(void)
{
    onewire_bus_handle_t bus = NULL;
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = ONEWIRE_BUS_GPIO,
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10,
    };
    ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));

    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_device;
    esp_err_t search_result = ESP_OK;

    ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
    ESP_LOGI(TAG, "Device iterator created, start searching...");
    do{
        search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
        if (search_result == ESP_OK)
        {
            ds18b20_config_t ds_cfg = {};
            if(ds18b20_new_device(&next_onewire_device, &ds_cfg, &s_ds18b20s[s_ds18b20_device_num]) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", s_ds18b20_device_num, next_onewire_device.address);
                s_ds18b20_device_num++;
            }
            else
            {
                ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
            }
        }
    }
    while(search_result != ESP_ERR_NOT_FOUND);
    ESP_ERROR_CHECK(onewire_del_device_iter(iter));
    ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", s_ds18b20_device_num);
}

void sensor_readTask(void *pvParameters)
{
    sensor_detect();
    while (1)
    {
        ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion(s_ds18b20s[0]));
        ESP_ERROR_CHECK(ds18b20_get_temperature(s_ds18b20s[0], &s_temperature));
        ESP_LOGI(TAG, "temperature read from DS18B20: %.2fC", s_temperature);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

void DS18B20_Init(void)
{
    xTaskCreate(&sensor_readTask, "sensor_readTask", 4096, NULL, 5, NULL);
}
