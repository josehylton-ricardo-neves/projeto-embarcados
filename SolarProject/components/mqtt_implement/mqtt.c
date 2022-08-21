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

#include "mqtt.h"
//#include "nvs_flash.h"
//#include "esp_wifi.h"
//#include "protocol_examples_common.h"

#define TAG "MQTT"

extern xSemaphoreHandle mqttConnectionSemaphore;

esp_mqtt_client_handle_t client;


//declaração da função que vai gerenciar/pegar tipos de eventos e fazer o tratamento
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event){

    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event->event_id) {
    case MQTT_EVENT_CONNECTED:
        msg_id = esp_mqtt_client_subscribe(client, "sis_embarcados_consumer", 0);
        xSemaphoreGive(mqttConnectionSemaphore);
        //ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        //msg_id = esp_mqtt_client_publish(client, "sis_embarcados_consumer", "valores do sensor", 0, 1, 0);
        //msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        //msg_id = esp_mqtt_client_subscribe(client, "sis_embarcados_consumer", 0);
        //msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        //msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        //msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        //ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        //ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        //msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        //ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        //ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG, "Message Received");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        sendMessage("sis_embarcados_consumer", "temperatura de 200 graus C");
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
            .host = "34.227.113.20",
            .port = 1883,
            //.uri = "mqtt://mqtt.eclipseprojects.io",
			//.uri = "mqtt://mqtt.eclipse.org",
	};

	//cadastra cliente mqtt e registrar o loop com event handle
	client = esp_mqtt_client_init(&mqtt_config);

	//registra loop de eventos(client, tipos de eventos que vai pegar noi handler,
	//nome do handler, passa client como arqgumento para capturar toda a estrutura)
	esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, client);

	//comando de inicializar a stack (dar start no mqtt)
	esp_mqtt_client_start(client);

}


void sendMessage(char * topic, char * message){
    int message_id = esp_mqtt_client_publish(client, topic, message, 0, 1, 0);
    ESP_LOGI(TAG, "message was sent successfully. ID: %d", message_id);
}
