#include <Module.h>

void debugPrint(const char *text)
{
#ifdef SERIAL_DEBUG
  Serial.println(text);
#endif
#ifdef TELNET_DEBUG
  telnet.println(text);
#endif
}

void debugPrintf(const char *format, ...)
{
  va_list args;
  va_start(args, format);

#if defined(SERIAL_DEBUG) || defined(TELNET_DEBUG)
  char buffer[128];
  vsnprintf(buffer, sizeof(buffer), format, args);
#endif
#ifdef SERIAL_DEBUG
  Serial.print(buffer);
#endif
#ifdef TELNET_DEBUG
  telnet.println(buffer);
#endif
  va_end(args);
}

void debugPrint(const IPAddress ip)
{
#ifdef SERIAL_DEBUG
  Serial.println(ip);
#endif
#ifdef TELNET_DEBUG
  telnet.println(ip);
#endif
}

double round2(double value)
{
  return (int)(value * 100 + 0.5) / 100.0;
}