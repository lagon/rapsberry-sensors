#ifndef __lagon_ina219_power_monitor_h__
#define __lagon_ina219_power_monitor_h__

#include <sys/types.h>
#include <stdio.h>
#include "i2clib.h"
#include <stdint.h>


struct ina219State {
	uint8_t address;
	int i2cBusDevice;
} ina219State;

struct ina219State *ina219_initPowerMonitor(uint8_t i2cBusID, uint8_t ina219Address);

void ina219_powerOn(struct ina219State *ina219);
void ina219_powerOff(struct ina219State *ina219);

void   ina219_initateVoltageReadingSingle(struct ina219State *ina219);

void ina219_setCalibrationRegister(struct ina219State *ina219);
int ina219_isReadingReady(struct ina219State *ina219);
int ina219_isReadingValid(struct ina219State *ina219);

double ina219_readBusVoltageSingle(struct ina219State *ina219);
double ina219_readShuntVoltageSingle(struct ina219State *ina219);

double ina219_readPowerSingle(struct ina219State *ina219);
double ina219_readCurrentSingle(struct ina219State *ina219);

void ina219_destroyPowerMonitorState(struct ina219State *ina219);

void ina219_testMeasurement();
#endif