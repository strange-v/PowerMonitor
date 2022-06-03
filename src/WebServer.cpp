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
    server.on("/power/api/chart", HTTP_GET, _getChartData);
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

void taskChartCalcs(void *pvParameters)
{
    time_t now;
    tm local;
    ChartData cd;

    for (;;)
    {
        xEventGroupWaitBits(eg, EVENT_UPDATE_CHART, pdTRUE, pdTRUE, portMAX_DELAY);

        time(&now);
        localtime_r(&now, &local);

        // ToDo: Optimize this
        cd.date = static_cast<uint8_t>(local.tm_mday);
        cd.date = cd.date << 8 | static_cast<uint8_t>(local.tm_hour);
        cd.date = cd.date << 8 | static_cast<uint8_t>(local.tm_min);

        float maxVoltage = tempData[0].voltage;
        float minVoltage = tempData[0].voltage;
        float maxPower = tempData[0].power;
        float minPower = tempData[0].power;
        for (decltype(tempData)::index_t i = 0; i < tempData.size(); i++)
        {
            if (maxVoltage < tempData[i].voltage)
                maxVoltage = tempData[i].voltage;
            if (minVoltage > tempData[i].voltage)
                minVoltage = tempData[i].voltage;
            
            if (maxPower < tempData[i].power)
                maxPower = tempData[i].power;
            if (minPower > tempData[i].power)
                minPower = tempData[i].power;
        }
        tempData.clear();

        cd.minVoltage = minVoltage;
        cd.maxVoltage = maxVoltage;
        cd.minPower = minPower;
        cd.maxPower = maxPower;
        
        if (xSemaphoreTake(semaHistoricalData, pdMS_TO_TICKS(10000)) == pdTRUE)
        {
            historicalData.push(cd);
            xSemaphoreGive(semaHistoricalData);
        }
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

void _getChartData(AsyncWebServerRequest *request)
{
    if(!request->hasParam("type"))
    {
        request->send(500, CONTENT_TYPE_TEXT, "No type");
        return;
    }

    bool needRetry = false;
    AsyncWebParameter* type = request->getParam("type");
    if (xSemaphoreTake(semaHistoricalData, TICKS_TO_WAIT0) == pdTRUE)
    {
        if (xSemaphoreTake(semaWebDataBuffer, TICKS_TO_WAIT0) == pdTRUE)
        {
            AsyncWebServerResponse *response = request->beginChunkedResponse(CONTENT_TYPE_JSON, [type](uint8_t *buffer, size_t maxLen, size_t index) -> size_t
            {
                char lineBuffer[64];
                size_t max = (maxLen > sizeof(webDataBuffer) ? sizeof(webDataBuffer) : maxLen) - 4;
                size_t copied = 0;
                size_t skipped = 0;
                size_t lineLen = 0;

                webDataBuffer[0] = '\0';
                if (index == 0)
                {
                    strcpy(webDataBuffer, "[");
                    copied++;
                }
                else
                    skipped++;

                for (decltype(historicalData)::index_t i = 0; i < historicalData.size(); i++)
                {
                    ChartData data = historicalData[i];
                    lineLen = sprintf(lineBuffer, "[%u,%.2f,%.2f],", data.date, _getMinValueByType(type, data), _getMaxValueByType(type, data));

                    if (index > 0 && skipped + lineLen <= index)
                    {
                        skipped += lineLen;
                        continue;
                    }

                    if (copied + lineLen > max)
                    {
                        memcpy(buffer, webDataBuffer, copied);
                        return copied;
                    }

                    strcat(webDataBuffer, lineBuffer);
                    copied += lineLen;
                }

                if (copied == 0 && index > 0)
                {
                    xSemaphoreGive(semaWebDataBuffer);
                    xSemaphoreGive(semaHistoricalData);
                    return 0;
                }

                if (strlen(webDataBuffer) > 1)
                    webDataBuffer[strlen(webDataBuffer) - 1] = '\0';
                else
                    copied++;

                strcat(webDataBuffer, "]");

                memcpy(buffer, webDataBuffer, copied);
                return copied;
            });
            request->send(response);
        }
        else
        {
            xSemaphoreGive(semaHistoricalData);
            needRetry = true;
        }
    }
    else
    {
        needRetry = true;
    }

    if (needRetry)
    {
        char buffer[64];
        sprintf(buffer, "%s?type=%s", request->url().c_str(), type->value().c_str());
        request->redirect(buffer);
    }
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
        sprintf(webDataBuffer, "HEAP: %d, NOW: %02d.%02d.%d %02d:%02d:%02d\n", ESP.getFreeHeap(), day, month, year, hour, minute, second);
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

float _getMinValueByType(AsyncWebParameter* param, ChartData data)
{
    return strcmp(param->value().c_str(), "1") == 0 ? data.minPower : data.minVoltage;
}

float _getMaxValueByType(AsyncWebParameter* param, ChartData data)
{
    return strcmp(param->value().c_str(), "1") == 0 ? data.maxPower : data.maxVoltage;
}
