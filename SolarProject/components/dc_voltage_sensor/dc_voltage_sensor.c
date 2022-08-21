#include <stdio.h>
#include "sensor_tensao_dc.h"
#include "driver/adc.h"

#define DC_VOLTAGE_SENSOR CONFIG_DC_VOLTAGE_SENSOR

void init_dc_voltage_sensor(){
	adc1_config_channel_atten(DC_VOLTAGE_SENSOR, ADC_ATTEN_DB_11);
}

float read_dc_voltage_sensor(){
	int Analog_battery_voltage = adc1_get_raw(DC_VOLTAGE_SENSOR);
	float battery_voltage = Analog_battery_voltage * (16.5/4095);
	return battery_voltage;
}
