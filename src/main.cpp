extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ETH.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <PZEM004Tv30.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <EEPROM.h>
#include <AsyncMqttClient.h>
#include <CircularBuffer.h>
#include <NodeData.h>
#include <ChartData.h>
#include <Network.h>
#include <Pzem.h>
#include <WebServer.h>
#include <Display.h>
#include <Mqtt.h>
#include <TimeSync.h>
#include <Module.h>
#include <Settings.h>
#include <Cfg.h>

void requestData();
void requestChartUpdate();

EventGroupHandle_t eg;
QueueHandle_t qMqtt;
TimerHandle_t tRequestData;
TimerHandle_t tConectMqtt;
TimerHandle_t tConectNetwork;
TimerHandle_t tHandleTimeSync;
TimerHandle_t tCleanupWebSockets;
TimerHandle_t tHandleChartCalcs;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, Cfg::pinSCL, Cfg::pinSDA);
PZEM004Tv30 pzem(Serial1, Cfg::pinRX, Cfg::pinTX);
SemaphoreHandle_t semaPzem;
AsyncWebServer server(80);
AsyncWebSocket ws("/power/ws");
AsyncMqttClient mqtt;
StaticJsonDocument<2048> webDoc;
char webDataBuffer[4096];
SemaphoreHandle_t semaWebDataBuffer;
CircularBuffer<ChartData, 720> historicalData;
SemaphoreHandle_t semaHistoricalData;
CircularBuffer<TempChartData, 60> tempData;
NodeData currentData;
Settings moduleSettings;

bool ethConnected = false;
bool timeSynchronized = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  eg = xEventGroupCreate();
  qMqtt = xQueueCreate(4, sizeof(MqttMessage));
  semaPzem = xSemaphoreCreateMutex();
  semaHistoricalData = xSemaphoreCreateMutex();
  semaWebDataBuffer = xSemaphoreCreateMutex();

  SPIFFS.begin(true);
  EEPROM.begin(sizeof(Settings));

  moduleSettings = getSettings();

  u8g2.begin();
  u8g2.setContrast(Cfg::screenBrightness);
  
  configureMqtt();
  WiFi.onEvent(WiFiEvent);
  ETH.begin();

  xTaskCreatePinnedToCore(taskRetrieveData, "RetrieveData", TaskStack10K, NULL, Priority3, NULL, Core1);
  xTaskCreatePinnedToCore(taskUpdateDisplay, "UpdateDisplay", TaskStack10K, NULL, Priority3, NULL, Core1);
  xTaskCreatePinnedToCore(taskUpdateWebClients, "UpdateWebClients", TaskStack10K, NULL, Priority3, NULL, Core1);
  xTaskCreatePinnedToCore(taskSendMqttMessages, "tMqtt", TaskStack10K, NULL, Priority2, NULL, Core1);
  xTaskCreatePinnedToCore(taskChartCalcs, "tChartCalcs", TaskStack10K, NULL, Priority2, NULL, Core1);
  tRequestData = xTimerCreate("RequestData", pdMS_TO_TICKS(moduleSettings.requestDataInterval), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(requestData));
  tConectMqtt = xTimerCreate("ConectMqtt", pdMS_TO_TICKS(10000), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  tConectNetwork = xTimerCreate("ConectNetwork", pdMS_TO_TICKS(20000), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(conectNetwork));
  tHandleTimeSync = xTimerCreate("HandleTimeSync", pdMS_TO_TICKS(10000), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(handleTimeSync));
  tCleanupWebSockets = xTimerCreate("CleanupWebSockets", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(cleanupWebSockets));
  tHandleChartCalcs = xTimerCreate("HandleChartCalcs", pdMS_TO_TICKS(60000), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(requestChartUpdate));

  initWebServer();

  xTimerStart(tRequestData, 0);
  xTimerStart(tConectNetwork, 0);
  xTimerStart(tCleanupWebSockets, 0);
}

void loop()
{
  if (ethConnected)
    ArduinoOTA.handle();
}

void requestData()
{
  xEventGroupSetBits(eg, EVENT_RETRIEVE_DATA);
}

void requestChartUpdate()
{
  xEventGroupSetBits(eg, EVENT_UPDATE_CHART);
}
