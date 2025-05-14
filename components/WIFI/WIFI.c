#include "WIFI.h"
static const char* TAG = "WiFi-Sta";
//static const uint8_t WIFI_AP_STARTED = 1;
static esp_netif_t *netif_handle_ap = NULL;
EventGroupHandle_t wifi_group;

static void netif_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT)
    {
        if(event_id == WIFI_EVENT_AP_STACONNECTED)
        {
            ESP_LOGI(
                        TAG, 
                        "Station "MACSTR" joined, AID=%d",
                        MAC2STR(((wifi_event_ap_stadisconnected_t *)event_data)->mac),
                        ((wifi_event_ap_stadisconnected_t *) event_data)->aid);
        }
        else if(event_id == WIFI_EVENT_AP_START)
        {
            ESP_LOGI(TAG, "AP Started!");
            //xEventGroupSetBits(wifi_group, WIFI_AP_STARTED);
        }
    } 
}
        

void wifi_init(void)
{
    nvs_flash_init();
    esp_event_loop_create_default();
    esp_netif_init();
    wifi_group = xEventGroupCreate();
    netif_handle_ap = esp_netif_create_default_wifi_ap();

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &netif_event_handler, NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    
    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = "ESP32_S3",
            .ssid_len = strlen("ESP32_S3"),
            .channel = 1,
            .password = "",
            .max_connection = 6,
            .authmode = WIFI_AUTH_OPEN, 
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config);
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_start();
}
