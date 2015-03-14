#include "actionQueue.h"

#ifndef __lagon_ina219_action_h__
#define __lagon_ina219_action_h__

extern const char* ina219PowerMonitorSensorName;
extern const char* ina219PowerMonitorSensorStateName;

extern const char *ina219LastVoltageMeasurementName;
extern const char *ina219LastCurrentMeasurementName;
extern const char *ina219LastPowerMeasurementName;
extern const char *ina219TotalPowerStoredMeasurementName;
extern const char *ina219TotelPowerConsumedMeasurementName;

long ina219_initActionFunction(GHashTable *sensorStatus);
long ina219_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus);
void ina219_closeActionFunction(GHashTable *sensorStatus);

#endif

