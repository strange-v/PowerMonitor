#include <Module.h>

void debugPrint(const char *text)
{
#ifdef M_DEBUG
  Serial.println(text);
#endif
}

void debugPrintf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
#ifdef M_DEBUG
  Serial.printf(format, args);
#endif
  va_end(args);
}

void debugPrint(const IPAddress ip)
{
#ifdef M_DEBUG
  Serial.println(ip);
#endif
}

double round2(double value)
{
  return (int)(value * 100 + 0.5) / 100.0;
}