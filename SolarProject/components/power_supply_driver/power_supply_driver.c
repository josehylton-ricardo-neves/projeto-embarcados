#include <stdio.h>
#include "driver/gpio.h"
#include "power_supply_driver.h"

#define AC_POWER_SUPPLY_DRIVER CONFIG_AC_POWER_SUPPLY_DRIVER
#define DC_POWER_SUPPLY_DRIVER CONFIG_DC_POWER_SUPPLY_DRIVER

void init_power_voltage_drivers(){

    gpio_reset_pin(AC_POWER_SUPPLY_DRIVER);
    gpio_set_direction(AC_POWER_SUPPLY_DRIVER, GPIO_MODE_INPUT_OUTPUT);
    gpio_pullup_dis(AC_POWER_SUPPLY_DRIVER);
    gpio_pulldown_dis(AC_POWER_SUPPLY_DRIVER);

    gpio_reset_pin(DC_POWER_SUPPLY_DRIVER);
    gpio_set_direction(DC_POWER_SUPPLY_DRIVER, GPIO_MODE_INPUT_OUTPUT);
    gpio_pullup_dis(DC_POWER_SUPPLY_DRIVER);
    gpio_pulldown_dis(DC_POWER_SUPPLY_DRIVER);
}

int get_ac_power_supply_driver(){
    return gpio_get_level(AC_POWER_SUPPLY_DRIVER);
}

void set_high_ac_power_supply_driver(){
    gpio_set_level(AC_POWER_SUPPLY_DRIVER, 1);
}

void set_low_ac_power_supply_driver(){
    gpio_set_level(AC_POWER_SUPPLY_DRIVER, 0);
}

int get_dc_power_supply_driver(){
    return gpio_get_level(DC_POWER_SUPPLY_DRIVER);
}

void set_high_dc_power_supply_driver(){
    gpio_set_level(DC_POWER_SUPPLY_DRIVER, 1);
}

void set_low_dc_power_supply_driver(){
    gpio_set_level(DC_POWER_SUPPLY_DRIVER, 0);
}
