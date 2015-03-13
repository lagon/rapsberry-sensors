#include "actionQueue.h"

#include <glib.h>

#ifndef __lagon_print_sensor_stat_h__
#define __lagon_print_sensor_stat_h__

extern const char *printSensorStatusName;
extern const long printSensorInterval;

struct printActionStatus {
	long last_printout;
} printActionStatus;

long print_initActionFunction(GHashTable *sensorStatus);
long print_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus);
void print_closeActionFunction(GHashTable *sensorStatus);


#endif