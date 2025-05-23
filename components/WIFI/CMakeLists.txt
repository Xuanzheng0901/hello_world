idf_component_register(SRCS "test_connect.c" "WIFI.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi nvs_flash HTTP)
