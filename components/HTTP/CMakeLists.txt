idf_component_register(SRCS "HTTP.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_server esp_netif freertos)
