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
#include <AsyncMqttClient.h>
#include <NodeData.h>
#include <Network.h>
#include <WebServer.h>
#include <Display.h>
#include <Mqtt.h>
#include <Cfg.h>

#define TaskStack10K 10000
#define TaskStack15K 15000
#define Priority1 1
#define Priority2 2
#define Priority3 3
#define Priority4 4
#define Priority5 5
#define Core0 0
#define Core1 1
#define TICKS_TO_WAIT12 12

void readData();

EventGroupHandle_t eg;
QueueHandle_t qData;
TimerHandle_t tReadData;
SemaphoreHandle_t sema_PZEM;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, Cfg::pinSCL, Cfg::pinSDA);
PZEM004Tv30 pzem(Serial1, Cfg::pinRX, Cfg::pinTX);
AsyncWebServer server(80);
AsyncWebSocket ws("/power/ws");
AsyncMqttClient mqtt;

bool ethConnected = false;
NodeData data;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  eg = xEventGroupCreate();
  qData = xQueueCreate(4, sizeof(MqttMessage));
  sema_PZEM = xSemaphoreCreateMutex();

  SPIFFS.begin(true);
  u8g2.begin();
  u8g2.setContrast(10);
  
  configureMqtt();
  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  
  ArduinoOTA.setPassword(Cfg::otaPassword);
  ArduinoOTA.begin();

  xTaskCreatePinnedToCore(taskUpdateDisplay, "UpdateDisplay", TaskStack15K, NULL, Priority3, NULL, Core1);
  xTaskCreatePinnedToCore(taskUpdateWebClients, "UpdateWebClients", TaskStack10K, NULL, Priority3, NULL, Core1);
  xTaskCreatePinnedToCore(taskSendMqttMessages, "tMqtt", TaskStack10K, NULL, Priority2, NULL, Core1);
  tReadData = xTimerCreate("ReadData", pdMS_TO_TICKS(Cfg::requestDataInterval), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(readData));

  initWebServer();

  xTimerStart(tReadData, 0);

  // pzem.resetEnergy();
}

void loop()
{
  ArduinoOTA.handle();
}

void readData()
{
  if (xSemaphoreTake(sema_PZEM, TICKS_TO_WAIT12) == pdTRUE)
  {
    data.voltage = pzem.voltage();
    data.current = pzem.current();
    data.power = pzem.power();
    data.energy = pzem.energy();
    data.frequency = pzem.frequency();
    data.pf = pzem.pf();
    data.uptime = millis() / 1000;

    if (Cfg::voltageMin > 0 && Cfg::voltageMax > 0)
      data.voltageWarn = data.voltage <= Cfg::voltageMin || data.voltage >= Cfg::voltageMax;
    if (Cfg::powerMax > 0)
      data.powerWarn = data.power >= Cfg::powerMax;
    if (Cfg::currentMax > 0)
      data.currentWarn = data.current >= Cfg::currentMax;

    xSemaphoreGive(sema_PZEM);

    xEventGroupSetBits(eg, EVENT_UPDATE_DISPLAY | EVENT_UPDATE_WEB_CLIENTS);
    
    MqttMessage msg = composeMessage(data);
    if (xQueueSendToBack(qData, &msg, 10) != pdPASS)
    {
      debugPrint("Failed to add to the mqtt queue");
    }
  }
}