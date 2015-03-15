#include "actionQueue.h"

#ifndef __lagon_mcp9808_action_h__
#define __lagon_mcp9808_action_h__

#include <glib.h>
#include <syslog.h>
#include <malloc.h>
#include "mcp9808_temperature.h"

extern const char* mcp9808TemperatureSensorName;
extern const char* mcp9808TemperatureSensorStateName;

extern const char *mcp9808TemperatureMeasurementName;

long mcp9808_initActionFunction(GHashTable *sensorStatus);
long mcp9808_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus);
void mcp9808_closeActionFunction(GHashTable *sensorStatus);

#endif

