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

extern EventGroupHandle_t eg;
extern QueueHandle_t qMqtt;
extern AsyncMqttClient mqtt;
extern TimerHandle_t tConectMqtt;
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