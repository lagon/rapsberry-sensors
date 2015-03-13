#include "actionQueue.h"

#include <glib.h>

#ifndef __lagon_save_actual_sensor_stat_h__
#define __lagon_save_actual_sensor_stat_h__

extern const char *saveActualSensorStatusName;
extern const long saveActualSensorInterval;

struct saveActualValueActionStatus {
	char *filename;
} saveActualValueActionStatus;

long save_actual_initActionFunction(GHashTable *sensorStatus);
long save_actual_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus);
void save_actual_closeActionFunction(GHashTable *sensorStatus);


#endif