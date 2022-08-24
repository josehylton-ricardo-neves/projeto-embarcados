#ifndef MQTT_H
#define MQTT_H


void mqtt_start();

void send_message(char * message);

void send_message_to_topic(char * topic, char * message);


#endif
