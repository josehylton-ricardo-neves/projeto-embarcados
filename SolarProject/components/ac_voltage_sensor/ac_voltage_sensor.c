#include <stdio.h>
#include "ac_voltage_sensor.h"
#include "driver/gpio.h"

#define AC_VOLTAGE_SENSOR CONFIG_AC_VOLTAGE_SENSOR


void init_ac_voltage_sensor() {
    gpio_reset_pin(AC_VOLTAGE_SENSOR);
    gpio_set_direction(AC_VOLTAGE_SENSOR, GPIO_MODE_INPUT);
    gpio_pullup_dis(AC_VOLTAGE_SENSOR);
    gpio_pulldown_dis(AC_VOLTAGE_SENSOR);
}

bool read_ac_voltage_sensor() {
    return gpio_get_level(AC_VOLTAGE_SENSOR);
}
