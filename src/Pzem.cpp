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

            if (Cfg::voltageMin > 0 && Cfg::voltageMax > 0)
                data.voltageWarn = data.voltage <= Cfg::voltageMin || data.voltage >= Cfg::voltageMax;
            if (Cfg::powerMax > 0)
                data.powerWarn = data.power >= Cfg::powerMax;
            if (Cfg::currentMax > 0)
                data.currentWarn = data.current >= Cfg::currentMax;

            xSemaphoreGive(sema_PZEM);

            xEventGroupSetBits(eg, EVENT_UPDATE_DISPLAY | EVENT_UPDATE_WEB_CLIENTS);

            MqttMessage msg = composeMessage(data);
            if (xQueueSendToBack(qMqtt, &msg, 10) != pdPASS)
            {
                debugPrint("Failed to add to the mqtt queue");
            }
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