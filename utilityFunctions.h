#ifndef __lagon_utility_functions_h__
#define __lagon_utility_functions_h__

#include <stdio.h>
#include <sys/time.h>
#include <malloc.h>
#include <string.h>
#include <syslog.h>

long long getCurrentUSecs();
void logErrorMessage(const char *format, const char *detail);

#endif