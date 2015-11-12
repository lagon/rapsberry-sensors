#ifndef __lagon_h21df_action_h__
#define __lagon_h21df_action_h__

#include "actionQueue.h"
#include "actionDescriptorStructure.h"
#include "sensorDescriptionStructure.h"

const extern long h21_measurement_offset;
extern struct actionDescriptorStructure_t h21dfActionStructure;

const extern char* h21dfTemperatureSensorName;
extern const char* h21dfHumiditySensorName;
extern const char* h21dfSensorStateName;

// long h21df_initActionFunction(GHashTable *sensorStatus);
// long h21df_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus);
// void h21df_closeActionFunction(GHashTable *sensorStatus);

#endif

