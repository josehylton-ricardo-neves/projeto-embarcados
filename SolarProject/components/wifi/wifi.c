#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

//#include "sdkconfig.h"

#include "wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define WIFI_SSID CONFIG_ESP_WIFI_SSID
#define WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define WIFI_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

#define TAG "Wifi"

static int s_retry_num = 0;

//
extern xSemaphoreHandle wifiConnectionSemaphore;

// handler do event groups (semaforo condicional)
static EventGroupHandle_t s_wifi_event_group;

//função que faz o tratamento dos eventos
static void event_handler (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

	if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	}else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if(s_retry_num < WIFI_MAXIMUM_RETRY){
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "Retry to connect to the AP");
		}else{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG, "Connect to the AP fail");
	}else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "Got IP (enderenço ip recebido):" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		xSemaphoreGive(wifiConnectionSemaphore);
	}
}

void wifi_start(){

	//inicializa o event_group (da os requisitos de aguardar pra saber se foi conectado)
	s_wifi_event_group = xEventGroupCreate();

	//inicialização da network interface
	ESP_ERROR_CHECK(esp_netif_init());

	//inicializar o loop do evento
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	//inicializar a network configuração padrão
	esp_netif_create_default_wifi_sta();

	//struct de configuração do wifi (inicializada com parametro padrão)
	wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();

	//usa a struct para inicializar o wifi
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));

	//registra os eventos do wifi (event_handler) (tipo de evento base q ta tratando, quais eventos,
	//endereço da função handler, argumento do evento)
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

	//registra eventos do network interface
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

	//sobreescreve a configuração do id e password do wifi, pois a mesma vem da configuração
	//do menu config
	wifi_config_t config = {
			.sta = {
					.ssid = WIFI_SSID,
					.password = WIFI_PASS
			},
	};

	//configura o modulo de inicialização do wifi
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	//configura o modulo do wifi com os parametros da struct config
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));

	//liga antena do wifi (antena do dispositivo)
	ESP_ERROR_CHECK(esp_wifi_start());


	//mostra se deu sucesso ou nao na conexão
	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
	            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
	            pdFALSE,
	            pdFALSE,
	            portMAX_DELAY);

	    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	     * happened. */
	    if (bits & WIFI_CONNECTED_BIT) {
	        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
	    } else if (bits & WIFI_FAIL_BIT) {
	        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", WIFI_SSID, WIFI_PASS);
	    } else {
	        ESP_LOGE(TAG, "UNEXPECTED EVENT");
	    }


	    //disvincula o event_handler
	    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
	    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
	    vEventGroupDelete(s_wifi_event_group);

}
