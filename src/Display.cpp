#include <Display.h>

void taskUpdateDisplay(void *pvParameters)
{
    for (;;)
    {
        xEventGroupWaitBits(eg, EVENT_UPDATE_DISPLAY, pdTRUE, pdTRUE, portMAX_DELAY);

        char buf[32];
        uint8_t y = 0;

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_fub11_tf);

        sprintf(buf, "%.0f V", data.voltage);
        y = 13;
        u8g2.drawStr(u8g2.getWidth() - u8g2.getStrWidth(buf), y, buf);
        u8g2.drawStr(0, y, "Voltage");

        if (data.power < 1000)
            sprintf(buf, "%.1f W", data.power);
        else
            sprintf(buf, "%.0f W", data.power);
        y = 30;
        u8g2.drawStr(u8g2.getWidth() - u8g2.getStrWidth(buf), y, buf);
        u8g2.drawStr(0, y, "Power");

        sprintf(buf, "%.2f A", data.current);
        y = 47;
        u8g2.drawStr(u8g2.getWidth() - u8g2.getStrWidth(buf), y, buf);
        u8g2.drawStr(0, y, "Current");

        sprintf(buf, "%.3f kWh", data.energy);
        y = 64;
        u8g2.drawStr((u8g2.getWidth() - u8g2.getStrWidth(buf)) / 2, y, buf);

        u8g2.sendBuffer();
    }
}
