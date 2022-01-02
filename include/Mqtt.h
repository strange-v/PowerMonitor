#ifndef MQTT_h
#define MQTT_h

#include <Arduino.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <Module.h>
#include <NodeData.h>
#include <Cfg.h>

#ifndef QUEUE_RECEIVE_DELAY
#define QUEUE_RECEIVE_DELAY 10
#endif

extern EventGroupHandle_t eg;
extern QueueHandle_t qData;
extern AsyncMqttClient mqtt;
extern bool ethConnected;
extern NodeData data;

struct MqttMessage
{
    char topic[16];
    char data[128];
};

void configureMqtt();
void connectToMqtt();
void taskSendMqttMessages(void *pvParameters);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
MqttMessage composeMessage(NodeData data);
uint16_t _mqttPublish(const char *topic, const char *data);

#endif