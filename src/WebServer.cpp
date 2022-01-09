#include <WebServer.h>

void initWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("/power/index.html"); });
    server.on("/power/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("/power/index.html"); });
    server.on("/power/api/settings", HTTP_GET, _getSettings);
    server.on("/power/api/settings", HTTP_PUT, [](AsyncWebServerRequest *request){}, NULL, _saveSettings);

    server.serveStatic("/power/", SPIFFS, "/");
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
        debugPrintf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        debugPrintf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        data[len] = 0;
        debugPrintf("WebSocket message: %s\n", data);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void _getSettings(AsyncWebServerRequest *request)
{
    StaticJsonDocument<512> doc;
    char buffer[512];

    Settings data = getSettings();

    doc["vMin"] = data.voltageMin;
    doc["vMax"] = data.voltageMax;
    doc["pMax"] = data.powerMax;
    doc["cMax"] = data.currentMax;
    
    doc["mqtt"] = data.enableMqtt;
    doc["mqttHost"] = data.mqttHost;
    doc["mqttPort"] = data.mqttPort;
    doc["mqttUsr"] = data.mqttUser;
    doc["mqttPwd"] = data.mqttPassword;
    doc["mqttTopic"] = data.mqttTopic;
    
    doc["rdi"] = data.requestDataInterval;
    doc["otaPwd"] = data.otaPassword;

    serializeJson(doc, buffer);

    request->send(200, CONTENT_TYPE_JSON, buffer);
}

void _saveSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    if (total > sizeof(dataBuffer))
    {
        request->send(500, CONTENT_TYPE_TEXT, "Content is to big");
        return;
    }

    memcpy(dataBuffer + index, data, len);

    if (len + index < total)
        return;

    StaticJsonDocument<512> doc;
    deserializeJson(doc, dataBuffer, total);

    Settings sett;
    sett.voltageMin = doc["vMin"];
    sett.voltageMax = doc["vMax"];
    sett.powerMax = doc["pMax"];
    sett.currentMax = doc["cMax"];

    sett.enableMqtt = doc["mqtt"];
    strlcpy(sett.mqttHost, doc["mqttHost"].as<const char*>(), sizeof(sett.mqttHost));
    sett.mqttPort = doc["mqttPort"];
    strlcpy(sett.mqttUser, doc["mqttUsr"].as<const char*>(), sizeof(sett.mqttUser));
    strlcpy(sett.mqttPassword, doc["mqttPwd"].as<const char*>(), sizeof(sett.mqttPassword));
    strlcpy(sett.mqttTopic, doc["mqttTopic"].as<const char*>(), sizeof(sett.mqttTopic));

    sett.requestDataInterval = doc["rdi"];
    strlcpy(sett.otaPassword, doc["otaPwd"].as<const char*>(), sizeof(sett.otaPassword));
    
    saveSettings(sett);
    request->send(200);

    bool restartRequired = moduleSettings.enableMqtt != sett.enableMqtt
        || strcmp(moduleSettings.mqttHost, sett.mqttHost) != 0
        || strcmp(moduleSettings.mqttUser, sett.mqttUser) != 0
        || strcmp(moduleSettings.mqttPassword, sett.mqttPassword) != 0
        || strcmp(moduleSettings.mqttTopic, sett.mqttTopic) != 0
        || strcmp(moduleSettings.otaPassword, sett.otaPassword) != 0;

    if (restartRequired)
    {
        ESP.restart();
    }

    moduleSettings = getSettings();
}

void _notFound(AsyncWebServerRequest *request)
{
    request->send(404, CONTENT_TYPE_TEXT, "Not found");
}