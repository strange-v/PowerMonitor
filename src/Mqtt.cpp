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

void connectToMqttTimerHandler()
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
            _mqttPublish(msg.topic, msg.data, msg.len, msg.retain);
        }
    }
}

void onMqttConnect(bool sessionPresent)
{
    xTimerStop(tConectMqtt, TICKS_TO_WAIT12);
    queueMqttDiscoveryMessages();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    xTimerStart(tConectMqtt, TICKS_TO_WAIT12);
}

void queueMqttDiscoveryMessages()
{
    if (!moduleSettings.enableHomeAssistant)
        return;

    char classMeasurement[] = "measurement";
    char classTotalIncreasing[] = "total_increasing";
    
    {
        MqttMessage msg = composeMqttDiscoveryMessage("Voltage", "V", classMeasurement, "{{ value_json.v | round(1) }}");
        xQueueSendToBack(qMqtt, &msg, 0);
    }
    {
        MqttMessage msg = composeMqttDiscoveryMessage("Frequency", "Hz", classMeasurement, "{{ value_json.f | round(0) }}");
        xQueueSendToBack(qMqtt, &msg, 0);
    }
    {
        MqttMessage msg = composeMqttDiscoveryMessage("Power", "W", classMeasurement, "{{ value_json.p | round(1) }}");
        xQueueSendToBack(qMqtt, &msg, 0);
    }
    {
        MqttMessage msg = composeMqttDiscoveryMessage("Current", "A", classMeasurement, "{{ value_json.c | round(2) }}");
        xQueueSendToBack(qMqtt, &msg, 0);
    }
    {
        MqttMessage msg = composeMqttDiscoveryMessage("Power Factor", "%", classMeasurement, "{{ (value_json.pf * 100) | round(0) }}");
        xQueueSendToBack(qMqtt, &msg, 0);
    }
    {
        MqttMessage msg = composeMqttDiscoveryMessage("Energy", "kWh", classTotalIncreasing, "{{ value_json.e | round(1) }}");
        xQueueSendToBack(qMqtt, &msg, 0);
    }
    {
        MqttMessage msg = composeMqttDiscoveryMessage("Uptime", "s", classTotalIncreasing, "{{ value_json.u }}");
        xQueueSendToBack(qMqtt, &msg, 0);
    }
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
    
    msg.retain = false;
    msg.len = strlen(msg.data);

    return msg;
}

MqttMessage composeMqttDiscoveryMessage(const char *name, const char *unit, const char *stateClass, const char *tpl)
{
    MqttMessage msg;
    StaticJsonDocument<512> doc;

    JsonObject dev  = doc.createNestedObject("dev");
    dev["name"] = Cfg::name;
    dev["sw"] = Cfg::version;
    dev["mf"] = Cfg::manufacturer;
    dev["mdl"] = Cfg::model;
    dev["ids"] = ETH.macAddress().c_str();

    String devCla = strcmp(name, "Uptime") == 0
        ? String("duration")
        : String(name);
    devCla.toLowerCase();
    devCla.replace(' ', '_');

    char uniqId[64];
    sprintf(uniqId, "%s_%s", ETH.macAddress().c_str(), devCla.c_str());

    doc["name"] = name;
    doc["uniq_id"] = uniqId;
    doc["stat_t"] = moduleSettings.mqttTopic;
    doc["stat_cla"] = stateClass;
    doc["unit_of_meas"] = unit;
    doc["dev_cla"] = devCla.c_str();
    doc["frc_upd"] = true;
    doc["val_tpl"] = tpl;

    sprintf(msg.topic, "%s/sensor/%s/config", moduleSettings.haDiscoveryPrefix, devCla.c_str());
    serializeJson(doc, msg.data, sizeof(msg.data));

    msg.retain = true;
    msg.len = strlen(msg.data);

    return msg;
}

uint16_t _mqttPublish(const char *topic, const char *data, size_t length, bool retain)
{
    return mqtt.publish(topic, 2, retain, data, length);
}
