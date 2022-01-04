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
}
#endif