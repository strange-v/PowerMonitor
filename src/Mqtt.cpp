#include <Mqtt.h>

void configureMqtt()
{
    if (!moduleSettings.enableMqtt)
        return;

    mqtt.onConnect(onMqttConnect);
    mqtt.onDisconnect(onMqttDisconnect);
    // mqtt.onMessage(onMqttMessage);
    mqtt.setServer(moduleSettings.mqttHost, moduleSettings.mqttPort);
    mqtt.setCredentials(moduleSettings.mqttUser, moduleSettings.mqttPassword);
}

void connectToMqtt()
{
    if (!moduleSettings.enableMqtt)
        return;
    
    if (ethConnected)
    {
        debugPrint("Connecting to MQTT...");
        mqtt.connect();
    }
    else
    {
        debugPrint("ETH is off, won't reconnect MQTT");
    }
}

void taskSendMqttMessages(void *pvParameters)
{
    MqttMessage msg;
    for (;;)
    {
        if (xQueueReceive(qMqtt, &msg, QUEUE_RECEIVE_DELAY))
        {
            _mqttPublish(msg.topic, msg.data);
        }
    }
}

void onMqttConnect(bool sessionPresent)
{
    xTimerStop(tConectMqtt, TICKS_TO_WAIT12);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    xTimerStart(tConectMqtt, TICKS_TO_WAIT12);
}

MqttMessage composeMqttMessage(NodeData data)
{
    MqttMessage msg;
    StaticJsonDocument<256> doc;

    doc["v"] = data.voltage;
    doc["f"] = data.frequency;
    doc["p"] = data.power;
    doc["c"] = data.current;
    doc["pf"] = data.pf;
    doc["e"] = data.energy;
    doc["u"] = data.uptime;

    serializeJson(doc, msg.data, sizeof(msg.data));
    strlcpy(msg.topic, moduleSettings.mqttTopic, sizeof(moduleSettings.mqttTopic));

    return msg;
}

uint16_t _mqttPublish(const char *topic, const char *data)
{
    return mqtt.publish(topic, 2, false, data);
}
