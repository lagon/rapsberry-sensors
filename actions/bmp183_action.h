#include "actionQueue.h"

#ifndef __lagon_bmp183_action_h__
#define __lagon_bmp183_action_h__

extern const char* bmp183PressureSensorName;
extern const char* bmp183PressureSensorStateName;

long bmp183_initActionFunction(GHashTable *sensorStatus);
long bmp183_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus);
void bmp183_closeActionFunction(GHashTable *sensorStatus);

#endif

