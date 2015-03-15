#ifndef __lagon_mcp9808_temperature_h__
#define __lagon_mcp9808_temperature_h__

#include <sys/types.h>
#include <stdio.h>
#include "i2clib.h"
#include <stdint.h>

extern const double mcp9808_critical_temperature;
extern const double mcp9808_upper_alert_boundary;
extern const double mcp9808_lower_alert_boundary;

struct mcp9808State {
	uint8_t address;
	int i2cBusDevice;
	uint16_t configuration;
} mcp9808State;

struct mcp9808State *mcp9808_initTemperatureSensor(uint8_t i2cBusID, uint8_t mcp9808Address);

void mcp9808_stopMeasuring(struct mcp9808State *mcp9808);
void mcp9808_startMeasuring(struct mcp9808State *mcp9808);

double mcp9880_readTemperature(struct mcp9808State *mcp9808);

int mcp9808_isAboveCritical(struct mcp9808State *mcp9808);
int mcp9808_isAboveUpperLimit(struct mcp9808State *mcp9808);
int mcp9808_isBelowLowerLimit(struct mcp9808State *mcp9808);

void mcp9808_destroyTemperatureSensorState(struct mcp9808State *mcp9808);

void mcp9808_testMeasurement();
#endif