#include <Network.h>

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_ETH_START:
    debugPrint("ETH Started");
    ETH.setHostname("ESP32-PM1");
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    debugPrint("ETH Connected");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    debugPrint("IP: ");
    debugPrint(ETH.localIP());

    ethConnected = true;
    connectToMqtt();
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    debugPrint("ETH Disconnected");
    ethConnected = false;
    break;
  case SYSTEM_EVENT_ETH_STOP:
    debugPrint("ETH Stopped");
    ethConnected = false;
    break;
  default:
    break;
  }
}
