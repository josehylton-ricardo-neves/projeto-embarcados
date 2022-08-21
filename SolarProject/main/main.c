#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "string.h"
#include "esp_system.h"
//#include <esp_log.h>

#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_log.h"
//#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

//#include "mqtt_client.h"

#include "sensor_tensao_dc.h"
//#include "ds18x20.h"
#include "ds18b20.h"

#include "wifi.h"
#include "mqtt.h"



#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

//static const char *TAG = "gpio_01";
//int delay_led_ms = 1000;

//DEFINI��O DE USO DOS PINOS INPUT
//#define SENSOR_TENSAO_DC_BATERIA  ADC1_CHANNEL_6
//#define SENSOR_CORRENTE_BATERIA  ADC1_CHANNEL_7
///#define SENSOR_TEMPERATURA  ADC1_CHANNEL_5


//DEFINI��O DE USO DOS PINOS OUTPUT
//#define RELE_REDE 7
//#define RELE_BATERIA 8


/*

// Temp Sensors are on GPIO26
#define TEMP_BUS GPIO_NUM_32



#define GPIO_DS18B20_0       (CONFIG_ONE_WIRE_GPIO)
#define MAX_DEVICES          (8)
#define DS18B20_RESOLUTION   (DS18B20_RESOLUTION_12_BIT)
#define SAMPLE_PERIOD        (1000)   // milliseconds

_Noreturn void app_main()
{
    // Override global log level
    esp_log_level_set("*", ESP_LOG_INFO);

    // To debug, use 'make menuconfig' to set default Log level to DEBUG, then uncomment:
    //esp_log_level_set("owb", ESP_LOG_DEBUG);
    //esp_log_level_set("ds18b20", ESP_LOG_DEBUG);

    // Stable readings require a brief period before communication
    vTaskDelay(2000.0 / portTICK_PERIOD_MS);

    // Create a 1-Wire bus, using the RMT timeslot driver
    OneWireBus * owb;
    owb_rmt_driver_info rmt_driver_info;
    owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb, true);  // enable CRC check for ROM code

    // Find all connected devices
    printf("Find devices:\n");
    OneWireBus_ROMCode device_rom_codes[MAX_DEVICES] = {0};
    int num_devices = 0;
    OneWireBus_SearchState search_state = {0};
    bool found = false;
    owb_search_first(owb, &search_state, &found);
    while (found)
    {
        char rom_code_s[17];
        owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
        printf("  %d : %s\n", num_devices, rom_code_s);
        device_rom_codes[num_devices] = search_state.rom_code;
        ++num_devices;
        owb_search_next(owb, &search_state, &found);
    }
    printf("Found %d device%s\n", num_devices, num_devices == 1 ? "" : "s");

    // In this example, if a single device is present, then the ROM code is probably
    // not very interesting, so just print it out. If there are multiple devices,
    // then it may be useful to check that a specific device is present.

    if (num_devices == 1)
    {
        // For a single device only:
        OneWireBus_ROMCode rom_code;
        owb_status status = owb_read_rom(owb, &rom_code);
        if (status == OWB_STATUS_OK)
        {
            char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
            owb_string_from_rom_code(rom_code, rom_code_s, sizeof(rom_code_s));
            printf("Single device %s present\n", rom_code_s);
        }
        else
        {
            printf("An error occurred reading ROM code: %d", status);
        }
    }
    else
    {
        // Search for a known ROM code (LSB first):
        // For example: 0x1502162ca5b2ee28
        OneWireBus_ROMCode known_device = {
            .fields.family = { 0x28 },
            .fields.serial_number = { 0xee, 0xb2, 0xa5, 0x2c, 0x16, 0x02 },
            .fields.crc = { 0x15 },
        };
        char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
        owb_string_from_rom_code(known_device, rom_code_s, sizeof(rom_code_s));
        bool is_present = false;

        owb_status search_status = owb_verify_rom(owb, known_device, &is_present);
        if (search_status == OWB_STATUS_OK)
        {
            printf("Device %s is %s\n", rom_code_s, is_present ? "present" : "not present");
        }
        else
        {
            printf("An error occurred searching for known device: %d", search_status);
        }
    }

    // Create DS18B20 devices on the 1-Wire bus
    DS18B20_Info * devices[MAX_DEVICES] = {0};
    for (int i = 0; i < num_devices; ++i)
    {
        DS18B20_Info * ds18b20_info = ds18b20_malloc();  // heap allocation
        devices[i] = ds18b20_info;

        if (num_devices == 1)
        {
            printf("Single device optimisations enabled\n");
            ds18b20_init_solo(ds18b20_info, owb);          // only one device on bus
        }
        else
        {
            ds18b20_init(ds18b20_info, owb, device_rom_codes[i]); // associate with bus and device
        }
        ds18b20_use_crc(ds18b20_info, true);           // enable CRC check on all reads
        ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
    }

//    // Read temperatures from all sensors sequentially
//    while (1)
//    {
//        printf("\nTemperature readings (degrees C):\n");
//        for (int i = 0; i < num_devices; ++i)
//        {
//            float temp = ds18b20_get_temp(devices[i]);
//            printf("  %d: %.3f\n", i, temp);
//        }
//        vTaskDelay(1000 / portTICK_PERIOD_MS);
//    }

    // Check for parasitic-powered devices
    bool parasitic_power = false;
    ds18b20_check_for_parasite_power(owb, &parasitic_power);
    if (parasitic_power) {
        printf("Parasitic-powered devices detected");
    }

    // In parasitic-power mode, devices cannot indicate when conversions are complete,
    // so waiting for a temperature conversion must be done by waiting a prescribed duration
    owb_use_parasitic_power(owb, parasitic_power);

#ifdef CONFIG_ENABLE_STRONG_PULLUP_GPIO
    // An external pull-up circuit is used to supply extra current to OneWireBus devices
    // during temperature conversions.
    owb_use_strong_pullup_gpio(owb, CONFIG_STRONG_PULLUP_GPIO);
#endif

    // Read temperatures more efficiently by starting conversions on all devices at the same time
    int errors_count[MAX_DEVICES] = {0};
    int sample_count = 0;
    if (num_devices > 0)
    {
        TickType_t last_wake_time = xTaskGetTickCount();

        while (1)
        {
            ds18b20_convert_all(owb);

            // In this application all devices use the same resolution,
            // so use the first device to determine the delay
            ds18b20_wait_for_conversion(devices[0]);

            // Read the results immediately after conversion otherwise it may fail
            // (using printf before reading may take too long)
            float readings[MAX_DEVICES] = { 0 };
            DS18B20_ERROR errors[MAX_DEVICES] = { 0 };

            for (int i = 0; i < num_devices; ++i)
            {
                errors[i] = ds18b20_read_temp(devices[i], &readings[i]);
            }

            // Print results in a separate loop, after all have been read
            printf("\nTemperature readings (degrees C): sample %d\n", ++sample_count);
            for (int i = 0; i < num_devices; ++i)
            {
                if (errors[i] != DS18B20_OK)
                {
                    ++errors_count[i];
                }

                printf("  %d: %.1f    %d errors\n", i, readings[i], errors_count[i]);
            }

            vTaskDelayUntil(&last_wake_time, SAMPLE_PERIOD / portTICK_PERIOD_MS);
        }
    }
    else
    {
        printf("\nNo DS18B20 devices detected!\n");
    }

    // clean up dynamically allocated data
    for (int i = 0; i < num_devices; ++i)
    {
        ds18b20_free(&devices[i]);
    }
    owb_uninitialize(owb);

    printf("Restarting now.\n");
    fflush(stdout);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
}
*/



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


void app_main(void) {
	//inicializa o nvs
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);


	//inicia semaforo
	wifiConnectionSemaphore = xSemaphoreCreateBinary();
	mqttConnectionSemaphore = xSemaphoreCreateBinary();

	wifi_start();

	xTaskCreate(&waitWifiConnection, "Processa WIFI", 4096, NULL, 1, NULL);
	xTaskCreate(&waitMqttConnection, "Processa MQTT", 4096, NULL, 1, NULL);


	ESP_LOGI("Main task", "saiu da task");


	//while(true) {

	//}
}



/*
void app_main(void){
	ESP_LOGI(TAG, "Iniciando o main_app");

	
	gpio_reset_pin(RELE_REDE);
	gpio_set_direction(RELE_REDE, GPIO_MODE_OUTPUT);
	//gpio_set_level(gpio_num, level)

	//quantidade de bits do ADC
	adc1_config_width(ADC_WIDTH_BIT_12);


	//adc1_config_channel_atten(SENSOR_TENSAO_DC_BATERIA, ADC_ATTEN_MAX);
	//adc1_config_channel_atten(SENSOR_TENSAO_DC_BATERIA, ADC_ATTEN_DB_11);

	//CONFIGURANDO O PINO PARA O SENSOR DE CORRENTE
	//adc1_config_channel_atten(SENSOR_CORRENTE_BATERIA, ADC_ATTEN_DB_11);
	sensor_tensao_dc_init(SENSOR_TENSAO_DC_BATERIA, ADC_ATTEN_DB_11);



	//CONFIGURANDO O PINO PARA O SENSOR DE TEMPERATURA
	//adc1_config_channel_atten(SENSOR_TEMPERATURA, ADC_ATTEN_DB_11);


	while(1){
	if(get_tensao_rede() != 0){
		if(rele_bateria = 1){
			gpio_set_level(rele_bateria, 0);
		}
		if(rele_rede = 0){
			gpio_set_level(rele_rede, 1);
		}
	}else{
		gpio_set_level(rele_rede, 0);
		if(get_temperatura_bateria() < 40 && get_tensao_bateria() > 11){
			if(rele_bateria == 0){
				gpio_set_level(rele_bateria, 1);
			}
		}else{
			gpio_set_level(rele_bateria, 0);
		}
	}

	}

	//get_tensao_rede();



    while (true) {
    	//ESP_LOGI(TAG, "O LED est� ON");
    	//vTaskDelay(1000 / portTICK_PERIOD_MS);
    	//sleep(1);

    	//int valorTensaoBateriaAnalog = adc1_get_raw(SENSOR_TENSAO_DC_BATERIA);
    	//float valorTensaoBateria = valorTensaoBateriaAnalog * (16.5/4095);
    	//valorTensaoBateria = valorTensaoBateria * (16.5/4095);

    	
    	int valorCorrenteBateriaAnalog = adc1_get_raw(SENSOR_CORRENTE_BATERIA);
    	valorCorrenteBateriaAnalog = valorCorrenteBateriaAnalog -495;
    	float valorCorrenteBateria = valorCorrenteBateriaAnalog * (3.3/4095);
    	

    	//medirTensaoDC(SENSOR_TENSAO_DC_BATERIA);
    	//printf("valor do sensor da bateria: %.2f", medirTensaoDC(SENSOR_TENSAO_DC_BATERIA));

    	//printf("valor analogico de tensao: %d valor de tensao: %.2f", valorTensaoBateriaAnalog, valorTensaoBateria);
    	//printf("\n");

    	//printf("valor analogico de corrente: %d valor de corrente: %.2f", valorCorrenteBateriaAnalog, valorCorrenteBateria);
    	//    	printf("\n");

    	//valorCorrenteBateriaAnalog = 0;

       // sleep(1);
    }
}
*/