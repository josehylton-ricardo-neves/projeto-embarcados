idf_component_register(SRCS "all_components_interface.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash wifi mqtt_implement ds18b20_interface acs712 dc_voltage_sensor
                    ac_voltage_sensor power_supply_driver)
