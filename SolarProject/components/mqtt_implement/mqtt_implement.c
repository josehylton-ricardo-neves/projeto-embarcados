#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt_implement.h"
#include "all_components_interface.h"

#define TAG "MQTT"
#define MQTT_HOST CONFIG_BROKER_HOST
#define MQTT_PORT CONFIG_BROKER_PORT
#define MQTT_URI CONFIG_BROKER_URL
#define MQTT_USERNAME CONFIG_BROKER_USERNAME
#define MQTT_PASSWORD CONFIG_BROKER_PASSWORD
#define MQTT_TOPIC_PUBLISHER CONFIG_BROKER_TOPIC_NAME_TO_PUBLISHER
#define MQTT_TOPIC_CONSUMER CONFIG_BROKER_TOPIC_NAME_TO_CONSUMER

extern xSemaphoreHandle mqttConnectionSemaphore;

esp_mqtt_client_handle_t client;


//declaração da função que vai gerenciar/pegar tipos de eventos e fazer o tratamento
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event){

    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event->event_id) {
    case MQTT_EVENT_CONNECTED:
        msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC_CONSUMER, 0);
        xSemaphoreGive(mqttConnectionSemaphore);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        break;
        
    case MQTT_EVENT_UNSUBSCRIBED:
        break;
    case MQTT_EVENT_PUBLISHED:
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        handle_mqtt_message(event->data_len, event->data);
        ESP_LOGI(TAG, "Message Received");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;

}

//assinatura padrão do handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    mqtt_event_handler_cb(event);
}


void mqtt_start() {

	esp_mqtt_client_config_t mqtt_config = {
            .host = MQTT_HOST,
            .port = MQTT_PORT,
            //.uri = "mqtt://mqtt.eclipseprojects.io",
			//.uri = "mqtt://mqtt.eclipse.org",
            //.username = "",
            //.password = "",
	};

	//cadastra cliente mqtt e registrar o loop com event handle
	client = esp_mqtt_client_init(&mqtt_config);

	//registra loop de eventos(client, tipos de eventos que vai pegar noi handler,
	//nome do handler, passa client como arqgumento para capturar toda a estrutura)
	esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, client);

	//comando de inicializar a stack (dar start no mqtt)
	esp_mqtt_client_start(client);

}

void send_message(char * message){
    send_message_to_topic(MQTT_TOPIC_PUBLISHER, message);
}

void send_message_to_topic(char * topic, char * message){
    int message_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    ESP_LOGI(TAG, "message was sent successfully. ID: %d", message_id);
}
