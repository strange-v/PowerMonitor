#ifndef WEB_SERVER_h
#define WEB_SERVER_h

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <NodeData.h>

#define EVENT_UPDATE_WEB_CLIENTS (1 << 20)

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern EventGroupHandle_t eg;
extern NodeData data;

void initWebServer();
void taskUpdateWebClients(void *pvParameters);
void _onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void _notFound(AsyncWebServerRequest *request);

#endif