#ifndef __lagon_tsl25911_luminosity_h__
#define __lagon_tsl25911_luminosity_h__

#include <sys/types.h>
#include <stdio.h>
#include "i2clib.h"
#include <stdint.h>

struct tsl25911State {
	uint8_t address;
	int i2cBusDevice;
} tsl25911State;

struct tsl25911State *tsl25911_initLuminositySensor(uint8_t i2cBusID, uint8_t tsl25911Address);

void tsl25911_enableMeasurement(struct tsl25911State *tsl25911);
void tsl25911_disableMeasurement(struct tsl25911State *tsl25911);
void tsl25911_setConfiguration(struct tsl25911State *tsl25911);
 tsl25911_getDeviceStatus(struct tsl25911State *tsl25911);

uint8_t tsl25911_verifyLuminositySensorPresent(struct tsl25911State *tsl25911);

void tsl25911_destroyTemperatureSensorState(struct tsl25911State *tsl25911);

void tsl25911_testMeasurement();
#endif