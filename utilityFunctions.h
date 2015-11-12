#ifndef __lagon_utility_functions_h__
#define __lagon_utility_functions_h__

#include <stdio.h>
#include <sys/time.h>
#include <malloc.h>
#include <string.h>
#include <syslog.h>

#include "sensorDescriptionStructure.h"

long long getCurrentUSecs();
void logErrorMessage(const char *format, const char *detail);

char *allocateAndConcatStrings(const char* str1, const char *str2);

struct allSensorsDescription_t *constructAllSensorDescription(const struct allSensorsDescription_t *original, char *nameAppendix);


#endif