#include <TimeSync.h>

void initTime()
{
    configTime(0, 0, moduleSettings.ntpServer);
    syncTime();
}

bool syncTime()
{
    time_t now;
    time(&now);

    tm local;
    localtime_r(&now, &local);
    timeSynchronized = local.tm_year > (2016 - 1900);
    return timeSynchronized;
}
