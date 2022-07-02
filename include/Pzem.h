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
#include <Display.h>
#include <WebServer.h>
#include <Mqtt.h>
#include <TimeSync.h>
#include <Settings.h>
#include <Cfg.h>

#define EVENT_RETRIEVE_DATA (1 << 1)
#define EVENT_RESET_ENERGY (1 << 2)

extern EventGroupHandle_t eg;
extern SemaphoreHandle_t semaPzem;
extern PZEM004Tv30 pzem;
extern TimerHandle_t tResetEnergy;

extern NodeData currentData;
extern Settings moduleSettings;

void taskRetrieveData(void *pvParameters);
void taskResetEnergy(void *pvParameters);
void resetEnergyTimerHandler();
bool resetEnergy();

#endif