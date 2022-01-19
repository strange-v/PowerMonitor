#include <WebServer.h>

void initWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("/power/index.html"); });
    server.on("/power/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("/power/index.html"); });
    server.on("/power/api/settings", HTTP_GET, _getSettings);
    server.on(
        "/power/api/settings", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, _saveSettings);
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

        doc["v"] = round2(data.voltage);
        doc["vw"] = data.voltageWarn;
        doc["f"] = round2(data.frequency);
        doc["p"] = round2(data.power);
        doc["pw"] = data.powerWarn;
        doc["c"] = round2(data.current);
        doc["cw"] = data.currentWarn;
        doc["pf"] = round2(data.pf);
        doc["e"] = round2(data.energy);
        doc["u"] = data.uptime;

        serializeJson(doc, buffer);

        ws.textAll(buffer);
    }
}

void cleanupWebSockets()
{
    ws.cleanupClients();
}

// ToDo: Should be a separate task
void handleChartCalcs()
{
    time_t now;
    tm local;
    time(&now);
    localtime_r(&now, &local);

    ChartData chartData;
    chartData.date = 0;
    // ToDo: Optimize this
    chartData.date = static_cast<uint8_t>(local.tm_mday);
    chartData.date = chartData.date << 8 | static_cast<uint8_t>(local.tm_hour);
    chartData.date = chartData.date << 8 | static_cast<uint8_t>(local.tm_min);

    float maxVoltage = tempChartBuffer[0].voltage;
    float minVoltage = tempChartBuffer[0].voltage;
    float maxPower = tempChartBuffer[0].power;
    float minPower = tempChartBuffer[0].power;
    using index_t = decltype(tempChartBuffer)::index_t;
    for (index_t i = 0; i < tempChartBuffer.size(); i++)
    {
        if (maxVoltage < tempChartBuffer[i].voltage)
            maxVoltage = tempChartBuffer[i].voltage;
        if (minVoltage > tempChartBuffer[i].voltage)
            minVoltage = tempChartBuffer[i].voltage;
        
        if (maxPower < tempChartBuffer[i].power)
            maxPower = tempChartBuffer[i].power;
        if (minPower > tempChartBuffer[i].power)
            minPower = tempChartBuffer[i].power;
    }
    tempChartBuffer.clear();

    chartData.minVoltage = round2(minVoltage);
    chartData.maxVoltage = round2(maxVoltage);
    chartData.minPower = round2(minPower);
    chartData.maxPower = round2(maxPower);
    chartBuffer.push(chartData);
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

    webDoc["rdi"] = data.requestDataInterval;
    webDoc["otaPwd"] = data.otaPassword;

    // AsyncResponseStream *response = request->beginResponseStream(CONTENT_TYPE_JSON);
    // serializeJson(webDoc, response);
    serializeJson(webDoc, webDataBuffer);
    request->send(200, CONTENT_TYPE_JSON, webDataBuffer);
}

void _saveSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    if (total > sizeof(webDataBuffer))
    {
        request->send(500, CONTENT_TYPE_TEXT, "Content is to big");
        return;
    }

    memcpy(webDataBuffer + index, data, len);

    if (len + index < total)
        return;

    webDoc.clear();
    deserializeJson(webDoc, webDataBuffer, total);

    Settings sett;
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

    sett.requestDataInterval = webDoc["rdi"];
    strlcpy(sett.otaPassword, webDoc["otaPwd"].as<const char *>(), sizeof(sett.otaPassword));

    saveSettings(sett);
    request->send(200);

    bool restartRequired = moduleSettings.enableMqtt != sett.enableMqtt || strcmp(moduleSettings.mqttHost, sett.mqttHost) != 0 || strcmp(moduleSettings.mqttUser, sett.mqttUser) != 0 || strcmp(moduleSettings.mqttPassword, sett.mqttPassword) != 0 || strcmp(moduleSettings.mqttTopic, sett.mqttTopic) != 0 || strcmp(moduleSettings.otaPassword, sett.otaPassword) != 0;

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
    }

    char lineBuffer[32];
    strcpy(webDataBuffer, "[");
    
    for (decltype(chartBuffer)::index_t i = 0; i < chartBuffer.size(); i++)
    {
        ChartData data = chartBuffer[i];
        sprintf(lineBuffer, "[%u,%.2f,%.2f],", data.date, _getMinValueByType(request->getParam("type"), data), _getMaxValueByType(request->getParam("type"), data));
        strcat(webDataBuffer, lineBuffer);
    }

    if (chartBuffer.size() > 0)
        webDataBuffer[strlen(webDataBuffer) - 1] = '\0';
    strcat(webDataBuffer, "]");

    // ToDo: Think about streaming data from the circular buffer directly into the response in a similar way
    // AsyncWebServerResponse *response = request->beginChunkedResponse(CONTENT_TYPE_JSON, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t
    // {
    //     uint16_t total = strlen(webDataBuffer);
    //     if (index >= total)
    //         return 0;

    //     size_t max = 2048;
    //     size_t maxLenToCopy = maxLen > max ? max : maxLen;
    //     size_t lenToCopy = total - index > maxLenToCopy ? maxLenToCopy : total - index;

    //     memcpy(buffer, webDataBuffer + index, lenToCopy);
    //     return lenToCopy;
    // });
    // request->send(response);
    
    request->send(200, CONTENT_TYPE_JSON, webDataBuffer);
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

    sprintf(webDataBuffer, "HEAP: %d, NOW: %02d.%02d.%d %02d:%02d:%02d\n", ESP.getFreeHeap(), day, month, year, hour, minute, second);
    request->send(200, CONTENT_TYPE_TEXT, webDataBuffer);
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
