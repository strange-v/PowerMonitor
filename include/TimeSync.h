#ifndef TIME_SYNC_h
#define TIME_SYNC_h

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <time.h>
#include <Arduino.h>

extern TimerHandle_t tHandleTimeSync;
extern TimerHandle_t tHandleChartCalcs;
extern bool ethConnected;
extern bool timeSynchronized;

void initTime();
void handleTimeSync();

#endif