#include "HTTP.h"

extern bool power_status[2], auto_control_volume;
extern float s_temperature;

extern const uint8_t index_html_start[] asm("_binary_index_html_gz_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_gz_end");

extern const uint8_t icon_start[] asm("_binary_favicon_ico_gz_start");
extern const uint8_t icon_end[]   asm("_binary_favicon_ico_gz_end");

static char* TAG = "HTTPD";
static httpd_handle_t server = NULL;

esp_err_t get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

esp_err_t get_power_info(httpd_req_t *req)
{
    char buf = (char)power_status[strcmp(req->uri, "/pwrinfo/+") == 0 ? 0 : 1] + '0';
    httpd_resp_send(req, &buf, 1);
    return ESP_OK;
}

esp_err_t get_temp_info(httpd_req_t *req)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", s_temperature);
    //ESP_LOGI("TEMP", "TEMP INFO GOT");
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_res(httpd_req_t *req)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", wiper_pos);
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_auto(httpd_req_t *req)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", auto_control_volume);
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t ico_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, (const char *)icon_start, icon_end - icon_start);
    return ESP_OK;
}

esp_err_t post_handler(httpd_req_t *req)
{
    char *buf = (char*)malloc(req->content_len*sizeof(char)+1);
    httpd_req_recv(req, buf, req->content_len);
    buf[req->content_len] = 0;

    ESP_LOGI(TAG, "Received data: %s", buf);
    uint8_t data = atoi(buf);
    set_resis_level(0xFF - data);
    free(buf);
    httpd_resp_send(req, "It's MyGO!!!!!", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t pwrctrl_post_handler(httpd_req_t *req)
{
    char *buf = (char*)malloc(req->content_len*sizeof(char)+1);
    httpd_req_recv(req, buf, req->content_len);
    buf[req->content_len] = 0;

    if(strcmp(req->uri, "/pwrctrl/+") == 0)
        Pwr_ctrl_P(atoi(buf));
    else if(strcmp(req->uri, "/pwrctrl/-") == 0)
        Pwr_ctrl_N(atoi(buf));

    // ESP_LOGI(TAG, "%s", buf);
    free(buf);
    httpd_resp_send(req, "Success", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t autoctrl_post_handler(httpd_req_t *req)
{
    char *buf = (char*)malloc(req->content_len*sizeof(char)+1);
    httpd_req_recv(req, buf, req->content_len);
    buf[req->content_len] = 0;

    auto_control_volume = atoi(buf);

    free(buf);
    httpd_resp_send(req, "Success", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void HTTP_Init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 10;
    config.uri_match_fn = httpd_uri_match_wildcard;
    ESP_LOGI(TAG, "Starting server on port: %d", config.server_port);
    ESP_ERROR_CHECK(httpd_start(&server, &config));
    const httpd_uri_t get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_handler,
        .user_ctx  = NULL
    };
    const httpd_uri_t get_icon = {
        .uri = "/favicon.ico",
        .method = HTTP_GET,
        .handler = ico_handler,
        .user_ctx  = NULL
    };
    const httpd_uri_t get_power = {
        .uri = "/pwrinfo/*",
        .method = HTTP_GET,
        .handler = get_power_info,
        .user_ctx  = NULL
    };
    const httpd_uri_t get_auto_status = {
        .uri = "/auto_status",
        .method = HTTP_GET,
        .handler = get_auto,
        .user_ctx  = NULL
    };
    const httpd_uri_t get_temp = {
        .uri = "/temp",
        .method = HTTP_GET,
        .handler = get_temp_info,
        .user_ctx  = NULL
    };
    const httpd_uri_t get_res_pos = {
        .uri = "/respos",
        .method = HTTP_GET,
        .handler = get_res,
        .user_ctx  = NULL
    };
    const httpd_uri_t post = {
        .uri = "/res",
        .method = HTTP_POST,
        .handler = post_handler,
        .user_ctx  = NULL
    };
    const httpd_uri_t post_pwr_ctr = {
        .uri = "/pwrctrl/*",
        .method = HTTP_POST,
        .handler = pwrctrl_post_handler,
        .user_ctx  = NULL
    };
    const httpd_uri_t auto_ctr = {
        .uri = "/auto",
        .method = HTTP_POST,
        .handler = autoctrl_post_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &get);
    httpd_register_uri_handler(server, &get_icon);
    httpd_register_uri_handler(server, &post);
    httpd_register_uri_handler(server, &get_power);
    httpd_register_uri_handler(server, &get_temp);
    httpd_register_uri_handler(server, &post_pwr_ctr);
    httpd_register_uri_handler(server, &get_res_pos);
    httpd_register_uri_handler(server, &auto_ctr);
    httpd_register_uri_handler(server, &get_auto_status);
}