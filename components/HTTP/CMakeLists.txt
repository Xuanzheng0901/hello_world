idf_component_register(SRCS "HTTPD.c" "HTTP.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_server esp_netif freertos Resistor Power_Control esp_http_client json
                    EMBED_FILES "./assets/index.html.gz" "./assets/favicon.ico.gz")
