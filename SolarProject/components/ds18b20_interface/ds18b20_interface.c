#include "ds18b20_interface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

#define GPIO_DS18B20_0       (CONFIG_DS18B20_ONE_WIRE_GPIO)

#define TAG "DS18B20"

OneWireBus * owb;
owb_rmt_driver_info rmt_driver_info;
OneWireBus_ROMCode device_rom_code;
OneWireBus_SearchState search_state;
bool found = false;
DS18B20_Info * device = 0;
DS18B20_ERROR error = 0;
float reading = 0;


void init_ds18b20_sensor() {
    
    owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb, true);

    owb_search_first(owb, &search_state, &found);

    char rom_code_s[17];
    owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
    device_rom_code = search_state.rom_code;
    owb_search_next(owb, &search_state, &found);

    DS18B20_Info * ds18b20_info = ds18b20_malloc();
    device = ds18b20_info;

    ds18b20_init_solo(ds18b20_info, owb);

    ds18b20_use_crc(ds18b20_info, true);
    ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_12_BIT);

}


float read_ds18b20_sensor() {

        ds18b20_convert_all(owb);
        ds18b20_wait_for_conversion(device);
        error = ds18b20_read_temp(device, &reading);

        return reading;

}