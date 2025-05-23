#include "WIFI.h"

static const char* TAG = "WIFI";
static esp_netif_t *sta_netif = NULL;
static SemaphoreHandle_t s_semph_get_ip6_addrs = NULL;
static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
static int s_retry_num = 0;
EventGroupHandle_t wifi_group;

const char *ipv6_type_table[6] = {
    "ESP_IP6_ADDR_IS_UNKNOWN",
    "ESP_IP6_ADDR_IS_GLOBAL",
    "ESP_IP6_ADDR_IS_LINK_LOCAL",
    "ESP_IP6_ADDR_IS_SITE_LOCAL",
    "ESP_IP6_ADDR_IS_UNIQUE_LOCAL",
    "ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6"
};

static void _wifi_on_disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void _wifi_on_connect_handler(void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void _ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static esp_err_t wifi_destroy(void);


static void _wifi_on_disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    s_retry_num++;
    if (s_retry_num > 5)
    {
        ESP_LOGI(TAG, "WiFi Connect failed %d times, stop reconnect.", s_retry_num);

        if (s_semph_get_ip_addrs)
            xSemaphoreGive(s_semph_get_ip_addrs);

        if (s_semph_get_ip6_addrs)
            xSemaphoreGive(s_semph_get_ip6_addrs);

        wifi_destroy();
        return;
    }
    wifi_event_sta_disconnected_t *disconn = event_data;
    if (disconn->reason == WIFI_REASON_ROAMING)
    {
        ESP_LOGD(TAG, "station roaming, do nothing");
        return;
    }
    ESP_LOGI(TAG, "Wi-Fi disconnected %d, trying to reconnect...", disconn->reason);
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED)
        return;

    ESP_ERROR_CHECK(err);
}

static void _wifi_on_connect_handler(void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    esp_netif_create_ip6_linklocal(esp_netif);
}

static void _ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if(event_id == IP_EVENT_STA_GOT_IP)
    {
        s_retry_num = 0;
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
        if (s_semph_get_ip_addrs)
            xSemaphoreGive(s_semph_get_ip_addrs);
        else
            ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
    }
    else if(event_id == IP_EVENT_GOT_IP6)
    {
        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;

        esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
        ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR ", type: %s", esp_netif_get_desc(event->esp_netif), IPV62STR(event->ip6_info.ip), ipv6_type_table[ipv6_type]);
        if (ipv6_type == ESP_IP6_ADDR_IS_GLOBAL)
        {
            if (s_semph_get_ip6_addrs)
                xSemaphoreGive(s_semph_get_ip6_addrs);
            else
                ESP_LOGI(TAG, "- IPv6 address: " IPV6STR ", type: %s", IPV62STR(event->ip6_info.ip), ipv6_type_table[ipv6_type]);
            xEventGroupSetBits(wifi_group, 2);
        }
    }
}

static esp_err_t wifi_destroy(void)
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &_wifi_on_disconnect_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &_ip_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &_wifi_on_connect_handler));

    if (s_semph_get_ip_addrs)
        vSemaphoreDelete(s_semph_get_ip_addrs);

    if (s_semph_get_ip6_addrs)
        vSemaphoreDelete(s_semph_get_ip6_addrs);

    return esp_wifi_disconnect();
}

static void wifi_start(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    esp_netif_config.route_prio = 128;
    sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static esp_err_t wifi_connect(wifi_config_t wifi_config)
{
    s_retry_num = 0;
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &_wifi_on_disconnect_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &_ip_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &_wifi_on_connect_handler, sta_netif));

    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }
    return ESP_OK;
}

esp_err_t wifi_init(void)
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_group = xEventGroupCreate();
    ESP_LOGI(TAG, "Start example_connect.");
    wifi_start();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "千早爱音の姛雷达",
            .password = "szx123456",
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
        }
    };
    return wifi_connect(wifi_config);
}