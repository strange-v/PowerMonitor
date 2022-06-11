#include <WebServer.h>

void initWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("/power/index.html"); });
    server.on("/power/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("/power/index.html"); });
    server.on("/power/api/settings", HTTP_GET, _getSettings);
    server.on("/power/api/settings", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, _saveSettings);
    server.on("/power/api/resetEnergy", HTTP_POST, _resetEnergy);
    server.on("/power/api/reboot", HTTP_GET, _reboot);
    server.on("/power/api/debug", HTTP_GET, _getDebug);

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

        doc["v"] = round2(currentData.voltage);
        doc["vw"] = currentData.voltageWarn;
        doc["f"] = round2(currentData.frequency);
        doc["p"] = round2(currentData.power);
        doc["pw"] = currentData.powerWarn;
        doc["c"] = round2(currentData.current);
        doc["cw"] = currentData.currentWarn;
        doc["pf"] = round2(currentData.pf);
        doc["e"] = round2(currentData.energy);
        doc["pe"] = round2(moduleSettings.prevEnergy);
        doc["u"] = currentData.uptime;

        serializeJson(doc, buffer);

        ws.textAll(buffer);
    }
}

void cleanupWebSocketsTimerHandler()
{
    ws.cleanupClients();
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
    webDoc.clear();
    Settings data = getSettings();

    webDoc["vMin"] = data.voltageMin;
    webDoc["vMax"] = data.voltageMax;
    webDoc["pMax"] = data.powerMax;
    webDoc["cMax"] = data.currentMax;

    webDoc["mqtt"] = data.enableMqtt;
    webDoc["mqttHost"] = data.mqttHost;
    webDoc["mqttPort"] = data.mqttPort;
    webDoc["mqttUsr"] = data.mqttUser;
    webDoc["mqttPwd"] = data.mqttPassword;
    webDoc["mqttTopic"] = data.mqttTopic;

    webDoc["ntp"] = data.ntpServer;
    webDoc["rdi"] = data.requestDataInterval;
    webDoc["otaPwd"] = data.otaPassword;
    webDoc["eRst"] = data.lastEnergyReset;

    if (xSemaphoreTake(semaWebDataBuffer, TICKS_TO_WAIT0) == pdTRUE)
    {
        serializeJson(webDoc, webDataBuffer);
        request->send(200, CONTENT_TYPE_JSON, webDataBuffer);
        
        xSemaphoreGive(semaWebDataBuffer);
    }
    else
    {
        request->redirect(request->url().c_str());
    }
}

void _saveSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    if (index == 0)
    {
        request->_tempObject = new uint8_t(0);
        if (xSemaphoreTake(semaWebDataBuffer, TICKS_TO_WAIT0) == pdTRUE)
            *(uint8_t*)request->_tempObject = 1;
        else
            request->redirect(request->url().c_str());
    }

    if (*(uint8_t*)request->_tempObject == 0)
        return;

    memcpy(webDataBuffer + index, data, len);

    if (len + index < total)
        return;

    webDoc.clear();
    deserializeJson(webDoc, webDataBuffer, total);

    Settings sett;
    sett.lastEnergyReset = moduleSettings.lastEnergyReset;
    sett.prevEnergy = moduleSettings.prevEnergy;

    sett.voltageMin = webDoc["vMin"];
    sett.voltageMax = webDoc["vMax"];
    sett.powerMax = webDoc["pMax"];
    sett.currentMax = webDoc["cMax"];

    sett.enableMqtt = webDoc["mqtt"];
    strlcpy(sett.mqttHost, webDoc["mqttHost"].as<const char *>(), sizeof(sett.mqttHost));
    sett.mqttPort = webDoc["mqttPort"];
    strlcpy(sett.mqttUser, webDoc["mqttUsr"].as<const char *>(), sizeof(sett.mqttUser));
    strlcpy(sett.mqttPassword, webDoc["mqttPwd"].as<const char *>(), sizeof(sett.mqttPassword));
    strlcpy(sett.mqttTopic, webDoc["mqttTopic"].as<const char *>(), sizeof(sett.mqttTopic));

    strlcpy(sett.ntpServer, webDoc["ntp"].as<const char *>(), sizeof(sett.ntpServer));
    sett.requestDataInterval = webDoc["rdi"];
    strlcpy(sett.otaPassword, webDoc["otaPwd"].as<const char *>(), sizeof(sett.otaPassword));

    saveSettings(sett);

    xSemaphoreGive(semaWebDataBuffer);
    request->send(200);

    bool restartRequired = moduleSettings.enableMqtt != sett.enableMqtt
        || strcmp(moduleSettings.mqttHost, sett.mqttHost) != 0
        || strcmp(moduleSettings.mqttUser, sett.mqttUser) != 0
        || strcmp(moduleSettings.mqttPassword, sett.mqttPassword) != 0
        || strcmp(moduleSettings.ntpServer, sett.ntpServer) != 0
        || strcmp(moduleSettings.mqttTopic, sett.mqttTopic) != 0
        || strcmp(moduleSettings.otaPassword, sett.otaPassword) != 0;

    if (restartRequired)
    {
        ESP.restart();
    }

    moduleSettings = getSettings();
}

void _resetEnergy(AsyncWebServerRequest *request)
{
#ifndef DO_NOT_RESET_ENERGY
    resetEnergy();
#endif
    request->send(200);
}

void _reboot(AsyncWebServerRequest *request)
{
    request->send(200, CONTENT_TYPE_TEXT, "OK");
    ESP.restart();
}

void _getDebug(AsyncWebServerRequest *request)
{
    time_t now;
    tm info;
    time(&now);
    localtime_r(&now, &info);

    int second = info.tm_sec;
    int minute = info.tm_min;
    int hour = info.tm_hour;
    int day = info.tm_mday;
    int month = info.tm_mon + 1;
    int year = info.tm_year + 1900;

    if (xSemaphoreTake(semaWebDataBuffer, TICKS_TO_WAIT0) == pdTRUE)
    {
        sprintf(webDataBuffer, "HEAP: %d, NOW: %02d.%02d.%d %02d:%02d:%02d\n",
            ESP.getFreeHeap(),
            day, month, year, hour, minute, second);
        request->send(200, CONTENT_TYPE_TEXT, webDataBuffer);
        
        xSemaphoreGive(semaWebDataBuffer);
    }
    else
    {
        request->redirect(request->url().c_str());
    }
}

void _notFound(AsyncWebServerRequest *request)
{
    request->send(404, CONTENT_TYPE_TEXT, "Not found");
}
