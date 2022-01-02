#ifndef NODE_DATA_h
#define NODE_DATA_h
#include <stdint.h>

struct NodeData
{
	float voltage;
	bool voltageWarn;
	float frequency;
	float power;
	bool powerWarn;
	float current;
	bool currentWarn;
	float energy;
	float pf;
	uint32_t uptime;
};

#endif