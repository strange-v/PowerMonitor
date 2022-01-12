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
#include <Settings.h>
#include <Module.h>
#include <Pzem.h>
#include <NodeData.h>

#define CONTENT_TYPE_TEXT "text/plain"
#define CONTENT_TYPE_HTML "text/html"
#define CONTENT_TYPE_CSS "text/css"
#define CONTENT_TYPE_JS "text/javascript"
#define CONTENT_TYPE_JSON "application/json"
#define EVENT_UPDATE_WEB_CLIENTS (1 << 20)

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern EventGroupHandle_t eg;
extern NodeData data;
extern Settings moduleSettings;
extern uint8_t dataBuffer[512];

void initWebServer();
void taskUpdateWebClients(void *pvParameters);
void _onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void _getSettings(AsyncWebServerRequest *request);
void _saveSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void _resetEnergy(AsyncWebServerRequest *request);
void _notFound(AsyncWebServerRequest *request);

#endif