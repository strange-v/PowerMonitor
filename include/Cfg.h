#ifndef CFG_h
#define CFG_h
#include <Arduino.h>

namespace Cfg
{
    const uint8_t pinSCL = 15;
    const uint8_t pinSDA = 4;
    const uint8_t pinRX = 14;
    const uint8_t pinTX = 17;

    const uint8_t screenBrightness = 1;

    const char name[] = "Power Monitor";
    const char manufacturer[] = "Just Testing";
    const char model[] = "PM1";
    const char version[] = "1.0.0";
}
#endif