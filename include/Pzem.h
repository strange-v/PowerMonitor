#ifndef PZEM_h
#define PZEM_h

#include <Arduino.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <PZEM004Tv30.h>
#include <CircularBuffer.h>
#include <NodeData.h>
#include <ChartData.h>
#include <Display.h>
#include <WebServer.h>
#include <Mqtt.h>
#include <Settings.h>
#include <Cfg.h>

#define EVENT_RETRIEVE_DATA (1 << 1)

extern EventGroupHandle_t eg;
extern SemaphoreHandle_t semaPzem;
extern PZEM004Tv30 pzem;
extern NodeData currentData;
extern Settings moduleSettings;
extern CircularBuffer<TempChartData, 60> tempData;

void taskRetrieveData(void *pvParameters);
bool resetEnergy();

#endif