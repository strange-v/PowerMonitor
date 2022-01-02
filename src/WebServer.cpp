#include <WebServer.h>

void initWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("/power/"); });
    server.on("/power/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });
    server.on("/power/main.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/main.css", "text/css"); });
    server.on("/power/main.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/main.js", "text/javascript"); });
    server.on("/power/service-worker.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/service-worker.js", "text/javascript"); });
    server.on("/power/manifest.json", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/manifest.json", "text/javascript"); });
    server.on("/power/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/favicon.ico", "text/javascript"); });
    server.serveStatic("/power/images", SPIFFS, "/images");
    server.onNotFound(_notFound);

    ws.onEvent(_onWebSocketEvent);
    server.addHandler(&ws);

    server.begin();
}

void taskUpdateWebClients(void *pvParameters)
{
    for (;;)
    {
        xEventGroupWaitBits(eg, EVENT_UPDATE_WEB_CLIENTS, pdTRUE, pdTRUE, portMAX_DELAY);

        StaticJsonDocument<256> doc;
        char buffer[256];

        doc["v"] = data.voltage;
        doc["vw"] = data.voltageWarn;
        doc["f"] = data.frequency;
        doc["p"] = data.power;
        doc["pw"] = data.powerWarn;
        doc["c"] = data.current;
        doc["cw"] = data.currentWarn;
        doc["pf"] = data.pf;
        doc["e"] = data.energy;
        doc["u"] = data.uptime;

        serializeJson(doc, buffer);

        ws.textAll(buffer);
    }
}

void _onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        data[len] = 0;
        Serial.printf("WebSocket message: %s\n", data);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void _notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}