idf_component_register(SRCS "ds18b20_interface.c"
                    INCLUDE_DIRS "include"
                    REQUIRES onewire_ds18b20 ds18b20)
