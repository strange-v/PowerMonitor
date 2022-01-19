#ifndef CHART_DATA_h
#define CHART_DATA_h
#include <stdint.h>

struct ChartData
{
    uint32_t date;
	float minVoltage;
	float maxVoltage;
	float minPower;
	float maxPower;
};

struct TempChartData
{
	float voltage;
	float power;
};

#endif