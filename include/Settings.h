#ifndef SETTINGS_h
#define SETTINGS_h

#include <Arduino.h>
#include <EEPROM.h>

#define START_ADDR 0
#define SIGNATURE 0x123455F0
#define DEFAULT_PORT 1883
#define DEFAULT_NTP_SERVER "pool.ntp.org"
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

    char ntpServer[32];
    uint16_t requestDataInterval;
    char otaPassword[32];
    uint32_t lastEnergyReset;
    float prevEnergy;
};

Settings getSettings();
void saveSettings(Settings newSettings);
Settings _getDefaultSettings();

#endif