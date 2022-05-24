#ifndef TIME_SYNC_h
#define TIME_SYNC_h

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <time.h>
#include <Arduino.h>
#include <Settings.h>


extern EventGroupHandle_t eg;
extern TimerHandle_t tResetEnergy;
extern TimerHandle_t tHandleChartCalcs;
extern Settings moduleSettings;
extern bool ethConnected;
extern bool timeSynchronized;

void initTime();
bool syncTime();

#endif