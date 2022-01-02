#include <Mqtt.h>

void configureMqtt()
{
    mqtt.onConnect(onMqttConnect);
    mqtt.onDisconnect(onMqttDisconnect);
    // mqtt.onMessage(onMqttMessage);
    mqtt.setServer(Cfg::mqttHost, Cfg::mqttPort);
    mqtt.setCredentials(Cfg::mqttUser, Cfg::mqttPassword);
}

void connectToMqtt()
{
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
        if (xQueueReceive(qData, &msg, QUEUE_RECEIVE_DELAY))
        {
            _mqttPublish(msg.topic, msg.data);
        }
    }
}

void onMqttConnect(bool sessionPresent)
{
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
}

MqttMessage composeMessage(NodeData data)
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
    memcpy(msg.topic, Cfg::mqttTopic, sizeof(Cfg::mqttTopic));

    return msg;
}

uint16_t _mqttPublish(const char *topic, const char *data)
{
    return mqtt.publish(topic, 2, false, data);
}