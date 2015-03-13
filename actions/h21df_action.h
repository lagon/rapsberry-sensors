#include "actionQueue.h"

#ifndef __lagon_h21df_action_h__
#define __lagon_h21df_action_h__

const extern long h21_measurement_offset;

const extern char* h21dfTemperatureSensorName;
extern const char* h21dfHumiditySensorName;
extern const char* h21dfSensorStateName;

long h21df_initActionFunction(GHashTable *sensorStatus);
long h21df_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus);
void h21df_closeActionFunction(GHashTable *sensorStatus);

#endif

