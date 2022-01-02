#ifndef CFG_h
#define CFG_h
#include <Arduino.h>

namespace Cfg
{
    const uint8_t pinSCL = 15;
    const uint8_t pinSDA = 4;
    const uint8_t pinRX = 14;
    const uint8_t pinTX = 17;

    //ToDo: Move everything below to EEPROM (VF)
    const char otaPassword[] = "123456";

    const uint64_t requestDataInterval = 1000;

    const uint16_t voltageMin = 198;
    const uint16_t voltageMax = 242;
    const uint16_t powerMax = 4800;
    const uint16_t currentMax = 0;

    const char mqttNodeId[] = "PM1";
    const IPAddress mqttHost(192, 168, 0, 100);
    const uint16_t mqttPort = 1883;
    const char mqttUser[] = "admin";
    const char mqttPassword[] = "123456";

	const char mqttTopic[] = "/pm1/values";
}
#endif