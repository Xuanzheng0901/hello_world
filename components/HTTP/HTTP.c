#include "HTTP.h"
#include "esp_http_client.h"
#include "lwip/netdb.h"
#include "cJSON.h"

extern char ipv6_addr[40];
extern EventGroupHandle_t wifi_group;

const char *cf_cert = "-----BEGIN CERTIFICATE-----\n"
"MIIDejCCAmKgAwIBAgIQf+UwvzMTQ77dghYQST2KGzANBgkqhkiG9w0BAQsFADBX\n"
"MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\n"
"CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIzMTEx\n"
"NTAzNDMyMVoXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\n"
"GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFI0\n"
"MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE83Rzp2iLYK5DuDXFgTB7S0md+8Fhzube\n"
"Rr1r1WEYNa5A3XP3iZEwWus87oV8okB2O6nGuEfYKueSkWpz6bFyOZ8pn6KY019e\n"
"WIZlD6GEZQbR3IvJx3PIjGov5cSr0R2Ko4H/MIH8MA4GA1UdDwEB/wQEAwIBhjAd\n"
"BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAd\n"
"BgNVHQ4EFgQUgEzW63T/STaj1dj8tT7FavCUHYwwHwYDVR0jBBgwFoAUYHtmGkUN\n"
"l8qJUC99BM00qP/8/UswNgYIKwYBBQUHAQEEKjAoMCYGCCsGAQUFBzAChhpodHRw\n"
"Oi8vaS5wa2kuZ29vZy9nc3IxLmNydDAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v\n"
"Yy5wa2kuZ29vZy9yL2dzcjEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqG\n"
"SIb3DQEBCwUAA4IBAQAYQrsPBtYDh5bjP2OBDwmkoWhIDDkic574y04tfzHpn+cJ\n"
"odI2D4SseesQ6bDrarZ7C30ddLibZatoKiws3UL9xnELz4ct92vID24FfVbiI1hY\n"
"+SW6FoVHkNeWIP0GCbaM4C6uVdF5dTUsMVs/ZbzNnIdCp5Gxmx5ejvEau8otR/Cs\n"
"kGN+hr/W5GvT1tMBjgWKZ1i4//emhA1JG1BbPzoLJQvyEotc03lXjTaCzv8mEbep\n"
"8RqZ7a2CPsgRbuvTPBwcOMBBmuFeU88+FSBX6+7iP0il8b4Z0QFqIwwMHfs/L6K1\n"
"vepuoxtGzi4CZ68zJpiq1UvSqTbFJjtbD4seiMHl\n"
"-----END CERTIFICATE-----";

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    return ESP_OK;
}

void update_dns_record(void)
{
    xEventGroupWaitBits(wifi_group, 2, pdTRUE, pdFALSE, portMAX_DELAY);
    char url[256];
    snprintf(url, sizeof(url), "https://api.cloudflare.com/client/v4/zones/%s/dns_records", zone_id);

    // 构造 JSON body
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "AAAA");
    cJSON_AddStringToObject(root, "name", "esp.xn--7br062ccfl55p.top");
    cJSON_AddStringToObject(root, "content", ipv6_addr);
    cJSON_AddNumberToObject(root, "ttl", 120);
    cJSON_AddBoolToObject(root, "proxied", false);
    char *post_data = cJSON_Print(root);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .cert_pem = cf_cert,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_PUT);
    esp_http_client_set_header(client, "Authorization", "Bearer " cf_token);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "User-Agent", "ESP32-DDNS/1.0");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        int content_length = esp_http_client_get_content_length(client);
        char *resp_body = malloc(content_length + 1);
        if (resp_body)
        {
            esp_http_client_read(client, resp_body, content_length);
            resp_body[content_length] = '\0';
            ESP_LOGI("CLOUDFLARE", "Response: %s", resp_body);
            free(resp_body);
        }
    }
    else
    {
        ESP_LOGE("CLOUDFLARE", "HTTP request failed: %s", esp_err_to_name(err));
    }

    cJSON_free(post_data);
    cJSON_Delete(root);
    esp_http_client_cleanup(client);
}
