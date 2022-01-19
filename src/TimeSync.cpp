#include <TimeSync.h>

void initTime()
{
    // ToDo: Move NTP server to config
    configTime(0, 0, "pool.ntp.org");
    xTimerStart(tHandleTimeSync, 0);
}

void handleTimeSync()
{
    if (!ethConnected)
        return;

    time_t now;
    time(&now);

    tm local;
    localtime_r(&now, &local);
    if (local.tm_year > (2016 - 1900))
    {
        timeSynchronized = true;
        xTimerStop(tHandleTimeSync, 0);
        xTimerStart(tHandleChartCalcs, 0);
    }
}