#include <stdio.h>
#include "HTTP.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_err.h"

static char* TAG = "HTTPD";
static httpd_handle_t server = NULL;
static uint8_t buffer[1024];

esp_err_t get_handler(httpd_req_t *req)
{
    httpd_resp_sendstr(req, "<!DOCTYPE html>\n<html>\n<head><meta charset=\"utf-8\"></head>\
    \n<body>\n<p>Hello World!<br>你好世界!</p>\n</body>\n</html>");
    return ESP_OK;
};

static const httpd_uri_t get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = buffer,
};

void HTTP_Init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    ESP_ERROR_CHECK(httpd_start(&server, &config));
    httpd_register_uri_handler(server, &get);
}