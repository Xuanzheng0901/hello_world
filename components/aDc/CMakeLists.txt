idf_component_register(SRCS "aDc.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_adc ST7789_Display)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)