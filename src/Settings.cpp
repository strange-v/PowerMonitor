#include <Settings.h>

Settings getSettings()
{
    Settings settings;
    EEPROM.readBytes(START_ADDR, &settings, sizeof(Settings));

    if (settings.signature != SIGNATURE)
    {
        settings = _getDefaultSettings();
        saveSettings(settings);
    }

    return settings;
}

void saveSettings(Settings newSettings)
{
    newSettings.signature = SIGNATURE;
    EEPROM.writeBytes(START_ADDR, &newSettings, sizeof(Settings));
    EEPROM.commit();
}

Settings _getDefaultSettings()
{
    Settings settings = {SIGNATURE, 0, 0, 0, 0, false};
    settings.requestDataInterval = 1000;
    settings.lastEnergyReset = 0;
    strlcpy(settings.otaPassword, DEFAULT_OTA_PWD, sizeof(DEFAULT_OTA_PWD));
    strlcpy(settings.ntpServer, DEFAULT_NTP_SERVER, sizeof(DEFAULT_NTP_SERVER));
    return settings;
}