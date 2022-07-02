#include <TimeSync.h>

void initTime()
{
    configTime(0, 0, moduleSettings.ntpServer);
    isTimeSynchronized();
}

bool isTimeSynchronized()
{
    time_t now;
    time(&now);

    tm local;
    localtime_r(&now, &local);

    return local.tm_year > (2016 - 1900);
}
