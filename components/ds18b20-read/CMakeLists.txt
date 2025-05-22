idf_component_register(SRCS "ds18b20-read.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ds18b20 onewire_bus)
