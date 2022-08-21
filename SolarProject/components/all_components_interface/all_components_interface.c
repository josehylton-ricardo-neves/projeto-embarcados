#include <stdio.h>
#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_event.h"
#include "nvs_flash.h"

#include "all_components_interface.h"







xSemaphoreHandle wifiConnectionSemaphore;
xSemaphoreHandle mqttConnectionSemaphore;


void waitWifiConnection(void * params) {

	while(true){
		if(xSemaphoreTake(wifiConnectionSemaphore, portMAX_DELAY)) {
			ESP_LOGI("Main task", "Conectou");
			mqtt_start();
		}
	}
}

void waitMqttConnection(void * params) {
	//char message[100];
	if(xSemaphoreTake(mqttConnectionSemaphore, portMAX_DELAY)){
		
		while(true) {
            vTaskDelay(10);
			//float temperatura = 20.0 + (float)rand()/(float)(RAND_MAX/10.0);
			//sprintf(message, "temperatura: %f", temperatura);

			//sendMessage("sis_embarcados_consumer", message);
			//vTaskDelay(3000 / portTICK_PERIOD_MS);
		}

	}
}


esp_err_t init_all_components() {

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

    init_ac_voltage_sensor();
    init_acs712();
    init_dc_voltage_sensor();
    init_ds18b20();




xSemaphoreHandle wifiConnectionSemaphore;
xSemaphoreHandle mqttConnectionSemaphore;
