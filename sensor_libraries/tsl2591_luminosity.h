#ifndef __lagon_tsl25911_luminosity_h__
#define __lagon_tsl25911_luminosity_h__

#include <sys/types.h>
#include <stdio.h>
#include "i2clib.h"
#include <stdint.h>

typedef enum {IT_100MS, IT_200MS, IT_300MS, IT_400MS, IT_500MS, IT_600MS} TSL2591_IntegrationTime_t;
typedef enum {Gain_Low, Gain_Medium, Gain_High, Gain_Max} TSL2591_SensitivityGain_t;

typedef enum {
	Low_100MS, Low_200MS, Low_300MS, Low_400MS, Low_500MS, Low_600MS, 
	Med_100MS, Med_200MS, Med_300MS, Med_400MS, Med_500MS, Med_600MS, 
	Hig_100MS, Hig_200MS, Hig_300MS, Hig_400MS, Hig_500MS, Hig_600MS, 
	Max_100MS, Max_200MS, Max_300MS, Max_400MS, Max_500MS, Max_600MS,
	ManuallySet} TSL2591_AutoSensitivityOrder_t;


struct tsl2591State {
	uint8_t address;
	int i2cBusDevice;
	TSL2591_IntegrationTime_t integrationTime;
	TSL2591_SensitivityGain_t sensitivityGain;
	TSL2591_AutoSensitivityOrder_t autoSensitivity;
} tsl2591State;

struct tsl2591State *tsl2591_initLuminositySensor(uint8_t i2cBusID, uint8_t tsl2591Address);
void tsl2591_enableMeasurement(struct tsl2591State *tsl2591);
void tsl2591_disableMeasurement(struct tsl2591State *tsl2591);
void tsl2591_setGain(struct tsl2591State *tsl2591, TSL2591_SensitivityGain_t gain);
void tsl2591_setIntegrationTime(struct tsl2591State *tsl2591, TSL2591_IntegrationTime_t integrationTime);

double tsl2591_readLuminosity(struct tsl2591State *tsl2591);
double tsl2591_readLuminosityWithAutoSensitivity(struct tsl2591State *tsl2591);

void tsl2591_destroyTemperatureSensorState(struct tsl2591State *tsl2591);


void tsl2591_testMeasurement();
#endif