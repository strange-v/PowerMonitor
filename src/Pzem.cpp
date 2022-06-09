#include <Pzem.h>

void taskRetrieveData(void *pvParameters)
{
    for (;;)
    {
        xEventGroupWaitBits(eg, EVENT_RETRIEVE_DATA, pdTRUE, pdTRUE, portMAX_DELAY);

        if (xSemaphoreTake(semaPzem, TICKS_TO_WAIT12) == pdTRUE)
        {
            uint32_t uptime = millis() / 1000;
            float voltage = pzem.voltage();
            if (isnan(voltage))
            {
                xSemaphoreGive(semaPzem);
                currentData = {0, false, 0, 0, false, 0, false, 0, 0, uptime};
                return;
            }

            currentData.voltage = voltage;
            currentData.current = pzem.current();
            currentData.power = pzem.power();
            currentData.energy = pzem.energy();
            currentData.frequency = pzem.frequency();
            currentData.pf = pzem.pf();

            xSemaphoreGive(semaPzem);

            currentData.uptime = uptime;
            currentData.voltageWarn = 0;
            currentData.powerWarn = 0;
            currentData.currentWarn = 0;

            if (moduleSettings.voltageMin > 0 && moduleSettings.voltageMax > 0)
                currentData.voltageWarn = currentData.voltage <= moduleSettings.voltageMin || currentData.voltage >= moduleSettings.voltageMax;
            if (moduleSettings.powerMax > 0)
                currentData.powerWarn = currentData.power >= moduleSettings.powerMax;
            if (moduleSettings.currentMax > 0)
                currentData.currentWarn = currentData.current >= moduleSettings.currentMax;

            tempData.push({currentData.voltage, currentData.power});
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
        }
    }
}

void taskResetEnergy(void *pvParameters)
{
    for (;;)
    {
        xEventGroupWaitBits(eg, EVENT_RESET_ENERGY, pdTRUE, pdTRUE, portMAX_DELAY);

        if (!timeSynchronized)
            continue;

        time_t now;
        tm local;

        time(&now);
        localtime_r(&now, &local);

        uint32_t exprectedDate = static_cast<uint16_t>(local.tm_year);
        exprectedDate = exprectedDate << 8 | static_cast<uint8_t>(local.tm_mon);
        
        if (moduleSettings.lastEnergyReset != exprectedDate)
        {
            moduleSettings.lastEnergyReset = exprectedDate;
            moduleSettings.prevEnergy = currentData.energy;
            resetEnergy();
            saveSettings(moduleSettings);
        }

        xTimerChangePeriod(tResetEnergy, pdMS_TO_TICKS(5 * 60 * 1000), portMAX_DELAY);
    }
}

void resetEnergyTimerHandler()
{
    xEventGroupSetBits(eg, EVENT_RESET_ENERGY);
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
