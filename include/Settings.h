#ifndef SETTINGS_h
#define SETTINGS_h

#include <Arduino.h>
#include <EEPROM.h>

#define START_ADDR 0
#define SIGNATURE 0x123456F0
#define DEFAULT_PORT 1883
#define DEFAULT_OTA_PWD "123456"

struct Settings
{
    uint32_t signature;

	uint16_t voltageMin;
	uint16_t voltageMax;
	uint16_t powerMax;
	uint16_t currentMax;
	
    bool enableMqtt;
    char mqttHost[32];
    uint16_t mqttPort;
    char mqttUser[32];
    char mqttPassword[32];
    char mqttTopic[32];

    uint16_t requestDataInterval;
    char otaPassword[32];
};

Settings getSettings();
void saveSettings(Settings newSettings);
Settings _getDefaultSettings();

#endif