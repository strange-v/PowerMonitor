#include <Network.h>

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_ETH_START:
    debugPrint("ETH Started");
    ETH.setHostname("ESP32-PM1");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    debugPrint("ETH Connected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    debugPrint("IP: ");
    debugPrint(ETH.localIP());

    ethConnected = true;
    initOta();
#ifdef TELNET_DEBUG
    telnet.begin();
#endif
    initTime();
    connectToMqttTimerHandler();
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    debugPrint("ETH Disconnected");
    ethConnected = false;
    break;
  case ARDUINO_EVENT_ETH_STOP:
    debugPrint("ETH Stopped");
    ethConnected = false;
    break;
  default:
    break;
  }
}

void conectNetworkTimerHandler()
{
  if (!ethConnected)
  {
    ETH.begin();
  }
}

void initOta()
{
  ArduinoOTA.setPassword(moduleSettings.otaPassword);
  ArduinoOTA.begin();
}

void taskHandleOta(void *pvParameters)
{
  for (;;)
  {
    if (ethConnected)
      ArduinoOTA.handle();
    
    delay(2000);
  }
}