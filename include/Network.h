#ifndef NETWORK_h
#define NETWORK_h

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <ETH.h>
#include <Module.h>
#include <Mqtt.h>

extern bool ethConnected;

void WiFiEvent(WiFiEvent_t event);
void conectNetwork();

#endif