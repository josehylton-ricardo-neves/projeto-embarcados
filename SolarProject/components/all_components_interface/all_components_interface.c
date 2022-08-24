#include <stdio.h>
#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include <string.h>

#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/adc.h"
#include "wifi.h"
#include "mqtt_implement.h"
#include "DS18B20_interface.h"
#include "acs712.h"
#include "dc_voltage_sensor.h"
#include "ac_voltage_sensor.h"
#include "power_supply_driver.h"

#include "all_components_interface.h"

#define MIN_BATTERY_VOLTAGE CONFIG_MIN_BATTERY_VOLTAGE
#define MAX_BATTERY_VOLTAGE CONFIG_MAX_BATTERY_VOLTAGE
#define MAX_BATTERY_TEMPERATURE CONFIG_MAX_BATTERY_TEMPERATURE

xSemaphoreHandle wifiConnectionSemaphore;
xSemaphoreHandle mqttConnectionSemaphore;

void waitWifiConnection(void * params) {
	while(true){
		if(xSemaphoreTake(wifiConnectionSemaphore, portMAX_DELAY)) {
			ESP_LOGI("*", "WIFI Connected \n");
			mqtt_start();
		}
	}
}

void waitMqttConnection(void * params) {
	if(xSemaphoreTake(mqttConnectionSemaphore, portMAX_DELAY)){
        ESP_LOGI("*", "MQTT Connected \n");
		while(true) {
			vTaskDelay(10);
		}
	}
}


void init_all_components() {

    esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
    ESP_ERROR_CHECK(ret);

    wifiConnectionSemaphore = xSemaphoreCreateBinary();
	mqttConnectionSemaphore = xSemaphoreCreateBinary();

    wifi_start();

    xTaskCreate(&waitWifiConnection, "WIFI task", 4096, NULL, 1, NULL);
	xTaskCreate(&waitMqttConnection, "MQTT task", 4096, NULL, 1, NULL);

	adc1_config_width(ADC_WIDTH_BIT_12);
    init_ac_voltage_sensor();
    init_dc_voltage_sensor();
    init_acs712_sensor();
    init_ds18b20_sensor();
	init_power_voltage_drivers();

}


//AC_VOLTAGE
bool read_ac_voltage(){
	return read_ac_voltage_sensor();
}

//TEMPERATURE
float read_temperature(){
	return read_ds18b20_sensor();
}

//DC_VOLTAGE
float read_dc_voltage(){
	return read_dc_voltage_sensor();
}

//BATERRY CURRENT
float read_baterry_current(){
	return read_acs712_sensor();
}

//DRIVERS
bool check_ac_power_supply(){
	if(get_ac_power_supply_driver() == 1){

		return true;
	}else {
		return false;
	}
}

bool check_dc_power_supply(){
	if(get_dc_power_supply_driver() == 1){
		return true;
	}else {
		return false;
	}
}

void enable_ac_power_supply(){
	set_high_ac_power_supply_driver();
}

void disable_ac_power_supply(){
	set_low_ac_power_supply_driver();
}

void enable_dc_power_supply(){
	set_high_dc_power_supply_driver();
}

void disable_dc_power_supply(){
	set_low_dc_power_supply_driver();
}

bool check_battery_conditions(){
	if(read_dc_voltage() > MIN_BATTERY_VOLTAGE && read_dc_voltage() < MAX_BATTERY_VOLTAGE && read_temperature() < MAX_BATTERY_TEMPERATURE){
		return true;
	}else{
		return false;
	}

}

void handle_mqtt_message(int tamanho, char * message){
	    char tratamento[tamanho +1];
		strcpy(tratamento, message);
		
        for (int i = 0; i <= tamanho; i++){
			if(i == tamanho){
				tratamento[i] = '\0';
			}
        }
	if(strcmp(tratamento, "/status") == 0){
		char tempe[20];
		char tensao[10];
		char corrent[10];
		float temperature = read_ds18b20_sensor();
		vTaskDelay(50 / portTICK_PERIOD_MS);
		sprintf (tempe, "%.1f", temperature);
		sprintf (tensao, "%.1f", read_dc_voltage());
		sprintf (corrent, "%.1f", read_baterry_current());
		char buffer[200] = "";

		strcat(buffer, "Temperatura: ");
		strcat(buffer, tempe);
		strcat(buffer, ", Tensao: ");
		strcat(buffer, tensao);
		strcat(buffer, ", Corrente: ");
		strcat(buffer, corrent);

		send_message(buffer);
	}
	else if(strcmp(tratamento, "/choose_battery") == 0){
		disable_ac_power_supply();
		enable_dc_power_supply();
	}

	else if(strcmp(tratamento, "/choose_mains") == 0){
		disable_dc_power_supply();
		enable_ac_power_supply();
	}else if(strcmp(tratamento, "/shutdown") == 0){
		disable_dc_power_supply();
		disable_ac_power_supply();
	}
	else {
		send_message("comando invalido");
	}
}