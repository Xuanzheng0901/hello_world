#include "WIFI.h"

static const char* TAG = "WiFi-Sta";
static esp_netif_t *netif_handle_sta = NULL, *netif_handle_ap = NULL;
char ipv6_addr[40];

EventGroupHandle_t wifi_group;

static void netif_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT)
        if(event_id == WIFI_EVENT_AP_STACONNECTED)
            ESP_LOGI(TAG, "Station "MACSTR" joined, AID=%d", MAC2STR(((wifi_event_ap_stadisconnected_t *)event_data)->mac), ((wifi_event_ap_stadisconnected_t *) event_data)->aid);
        else if (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED)
            esp_wifi_connect();
        else if(event_id == WIFI_EVENT_STA_CONNECTED)
            esp_netif_create_ip6_linklocal(netif_handle_sta);

    else if(event_base == IP_EVENT) //打印ip
        if(event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *data = (ip_event_got_ip_t *)event_data;
            ESP_LOGW(TAG, "Got IPv4 address:"IPSTR, IP2STR(&data->ip_info.ip));
        }
        else if(event_id == IP_EVENT_GOT_IP6)
        {
            ip_event_got_ip6_t *data6 = (ip_event_got_ip6_t *)event_data;
            esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&data6->ip6_info.ip);
            if(ipv6_type == ESP_IP6_ADDR_IS_GLOBAL)
            {
                snprintf(ipv6_addr, sizeof(ipv6_addr), IPV6STR, IPV62STR(data6->ip6_info.ip));
                xEventGroupSetBits(wifi_group, 2);
                ESP_LOGI(TAG, "%s", ipv6_addr);
            }
        }

}

void wifi_sta_init(void)
{
    netif_handle_sta = esp_netif_create_default_wifi_sta();
    wifi_config_t config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PWD,
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        }
    };
    esp_wifi_set_config(WIFI_IF_STA, &config);
}

void wifi_ap_init(void)
{
    netif_handle_ap = esp_netif_create_default_wifi_ap();
    wifi_config_t config = {
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
    esp_wifi_set_config(WIFI_IF_AP, &config);
}

void WIFI_Init(void)
{
    nvs_flash_init();

    esp_event_loop_create_default();
    wifi_group = xEventGroupCreate();
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &netif_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &netif_event_handler, NULL);

    esp_netif_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_sta_init();
    wifi_ap_init();

    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_netif_set_default_netif(netif_handle_sta);
    esp_wifi_start();
}
