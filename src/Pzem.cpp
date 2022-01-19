#include <Pzem.h>

void taskRetrieveData(void *pvParameters)
{
    for (;;)
    {
        xEventGroupWaitBits(eg, EVENT_RETRIEVE_DATA, pdTRUE, pdTRUE, portMAX_DELAY);

        if (xSemaphoreTake(sema_PZEM, TICKS_TO_WAIT12) == pdTRUE)
        {
            data.voltage = pzem.voltage();
            data.current = pzem.current();
            data.power = pzem.power();
            data.energy = pzem.energy();
            data.frequency = pzem.frequency();
            data.pf = pzem.pf();
            data.uptime = millis() / 1000;

            if (moduleSettings.voltageMin > 0 && moduleSettings.voltageMax > 0)
                data.voltageWarn = data.voltage <= moduleSettings.voltageMin || data.voltage >= moduleSettings.voltageMax;
            if (moduleSettings.powerMax > 0)
                data.powerWarn = data.power >= moduleSettings.powerMax;
            if (moduleSettings.currentMax > 0)
                data.currentWarn = data.current >= moduleSettings.currentMax;

            xSemaphoreGive(sema_PZEM);

            xEventGroupSetBits(eg, EVENT_UPDATE_DISPLAY | EVENT_UPDATE_WEB_CLIENTS);

            // ToDo: Probably move to a separate task
            if (moduleSettings.enableMqtt)
            {
                MqttMessage msg = composeMqttMessage(data);
                if (xQueueSendToBack(qMqtt, &msg, QUEUE_RECEIVE_DELAY) != pdPASS)
                {
                    debugPrint("Failed to add to the mqtt queue");
                }
            }

            // ToDo: Probably move to a separate task
            tempChartBuffer.push({data.voltage, data.power});
        }
    }
}

bool resetEnergy()
{
    bool result = false;
    if (xSemaphoreTake(sema_PZEM, TICKS_TO_WAIT12) == pdTRUE)
    {
        result = pzem.resetEnergy();
        xSemaphoreGive(sema_PZEM);
    }
    return result;
}