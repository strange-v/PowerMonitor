#ifndef PZEM_h
#define PZEM_h

#include <Arduino.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <PZEM004Tv30.h>
#include <NodeData.h>
#include <Display.h>
#include <WebServer.h>
#include <Mqtt.h>
#include <Settings.h>
#include <Cfg.h>

#define EVENT_RETRIEVE_DATA (1 << 1)

extern EventGroupHandle_t eg;
extern SemaphoreHandle_t sema_PZEM;
extern PZEM004Tv30 pzem;
extern NodeData data;
extern Settings moduleSettings;

void taskRetrieveData(void *pvParameters);
bool resetEnergy();

#endif