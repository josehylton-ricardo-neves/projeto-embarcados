#include <stdio.h>
#include "acs712.h"
#include "driver/adc.h"

#define ACS712_ADC_CHANNEL CONFIG_ACS712_ADC_CHANNEL

int init = 0;
int zero = 0;

void calibrate_sensor(){
    zero = adc1_get_raw(ACS712_ADC_CHANNEL);
}

void init_acs712_sensor(){
	adc1_config_channel_atten(ACS712_ADC_CHANNEL, ADC_ATTEN_DB_11);
    init = 1;
    calibrate_sensor();
}

float read_acs712_sensor(){
    if(init == 1) {
        int measure_value = adc1_get_raw(ACS712_ADC_CHANNEL);
        uint8_t current_value = measure_value - zero;
        float current = (current_value * 3.3) / (4095 * 0.066);
        return current;
    } else {
        return 0;
    }   
}

