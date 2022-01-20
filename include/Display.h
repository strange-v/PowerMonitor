#ifndef DISPLAY_h
#define DISPLAY_h

#include <Arduino.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <U8g2lib.h>
#include <NodeData.h>

#define EVENT_UPDATE_DISPLAY (1 << 10)

extern EventGroupHandle_t eg;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern NodeData currentData;

void taskUpdateDisplay(void *pvParameters);

#endif