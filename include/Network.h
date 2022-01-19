#ifndef NETWORK_h
#define NETWORK_h

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <ArduinoOTA.h>
#include <ETH.h>
#include <Module.h>
#include <Mqtt.h>
#include <TimeSync.h>
#include <Settings.h>

extern bool ethConnected;
extern Settings moduleSettings;

void WiFiEvent(WiFiEvent_t event);
void conectNetwork();
void initOta();

#endif