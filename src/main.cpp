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
#include <NodeData.h>
#include <Network.h>
#include <Pzem.h>
#include <WebServer.h>
#include <Display.h>
#include <Mqtt.h>
#include <Module.h>
#include <Settings.h>
#include <Cfg.h>

void requestData();

EventGroupHandle_t eg;
SemaphoreHandle_t sema_PZEM;
QueueHandle_t qMqtt;
TimerHandle_t tRequestData;
TimerHandle_t tConectMqtt;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, Cfg::pinSCL, Cfg::pinSDA);
PZEM004Tv30 pzem(Serial1, Cfg::pinRX, Cfg::pinTX);
AsyncWebServer server(80);
AsyncWebSocket ws("/power/ws");
AsyncMqttClient mqtt;

bool ethConnected = false;
Settings moduleSettings;
NodeData data;
uint8_t dataBuffer[512];

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  eg = xEventGroupCreate();
  qMqtt = xQueueCreate(4, sizeof(MqttMessage));
  sema_PZEM = xSemaphoreCreateMutex();

  SPIFFS.begin(true);
  EEPROM.begin(sizeof(Settings));

  moduleSettings = getSettings();

  u8g2.begin();
  u8g2.setContrast(Cfg::screenBrightness);
  
  configureMqtt();
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  
  ArduinoOTA.setPassword(moduleSettings.otaPassword);
  ArduinoOTA.begin();

  xTaskCreatePinnedToCore(taskRetrieveData, "RetrieveData", TaskStack10K, NULL, Priority3, NULL, Core1);
  xTaskCreatePinnedToCore(taskUpdateDisplay, "UpdateDisplay", TaskStack15K, NULL, Priority3, NULL, Core1);
  xTaskCreatePinnedToCore(taskUpdateWebClients, "UpdateWebClients", TaskStack10K, NULL, Priority3, NULL, Core1);
  xTaskCreatePinnedToCore(taskSendMqttMessages, "tMqtt", TaskStack10K, NULL, Priority2, NULL, Core1);
  tRequestData = xTimerCreate("RequestData", pdMS_TO_TICKS(moduleSettings.requestDataInterval), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(requestData));
  tConectMqtt = xTimerCreate("ConectMqtt", pdMS_TO_TICKS(10000), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));

  initWebServer();

  xTimerStart(tRequestData, 0);
}

void loop()
{
  ArduinoOTA.handle();
  //ToDo: Do this in timer
  ws.cleanupClients();
}

void requestData()
{
  xEventGroupSetBits(eg, EVENT_RETRIEVE_DATA);
}