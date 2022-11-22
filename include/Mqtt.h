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
#include <ETH.h>
#include <Module.h>
#include <NodeData.h>
#include <Settings.h>
#include <Cfg.h>

extern EventGroupHandle_t eg;
extern QueueHandle_t qMqtt;
extern AsyncMqttClient mqtt;
extern TimerHandle_t tConectMqtt;
extern bool ethConnected;
extern NodeData currentData;
extern Settings moduleSettings;

struct MqttMessage
{
    char topic[64];
    char data[512];
};

void configureMqtt();
void connectToMqttTimerHandler();
void taskSendMqttMessages(void *pvParameters);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void queueMqttDiscoveryMessages();
MqttMessage composeMqttMessage(NodeData data);
MqttMessage composeMqttDiscoveryMessage(const char *name, const char *unit, const char *tpl);
uint16_t _mqttPublish(const char *topic, const char *data);

#endif