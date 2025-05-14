idf_component_register(SRCS "aDc.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_adc OLED Power_Control)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)