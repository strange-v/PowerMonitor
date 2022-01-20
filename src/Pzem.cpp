#include <Pzem.h>

void taskRetrieveData(void *pvParameters)
{
    for (;;)
    {
        xEventGroupWaitBits(eg, EVENT_RETRIEVE_DATA, pdTRUE, pdTRUE, portMAX_DELAY);

        if (xSemaphoreTake(semaPzem, TICKS_TO_WAIT12) == pdTRUE)
        {
            currentData.voltage = pzem.voltage();
            currentData.current = pzem.current();
            currentData.power = pzem.power();
            currentData.energy = pzem.energy();
            currentData.frequency = pzem.frequency();
            currentData.pf = pzem.pf();
            currentData.uptime = millis() / 1000;

            if (moduleSettings.voltageMin > 0 && moduleSettings.voltageMax > 0)
                currentData.voltageWarn = currentData.voltage <= moduleSettings.voltageMin || currentData.voltage >= moduleSettings.voltageMax;
            if (moduleSettings.powerMax > 0)
                currentData.powerWarn = currentData.power >= moduleSettings.powerMax;
            if (moduleSettings.currentMax > 0)
                currentData.currentWarn = currentData.current >= moduleSettings.currentMax;

            xSemaphoreGive(semaPzem);

            xEventGroupSetBits(eg, EVENT_UPDATE_DISPLAY | EVENT_UPDATE_WEB_CLIENTS);

            // ToDo: Probably move to a separate task
            if (moduleSettings.enableMqtt)
            {
                MqttMessage msg = composeMqttMessage(currentData);
                if (xQueueSendToBack(qMqtt, &msg, QUEUE_RECEIVE_DELAY) != pdPASS)
                {
                    debugPrint("Failed to add to the mqtt queue");
                }
            }

            tempData.push({currentData.voltage, currentData.power});
        }
    }
}

bool resetEnergy()
{
    bool result = false;
    if (xSemaphoreTake(semaPzem, TICKS_TO_WAIT12) == pdTRUE)
    {
        result = pzem.resetEnergy();
        xSemaphoreGive(semaPzem);
    }
    return result;
}