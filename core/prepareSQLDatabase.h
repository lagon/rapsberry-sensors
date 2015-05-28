#ifndef __lagon_prepare_sql_database__
#define __lagon_prepare_sql_database__

#include <glib.h>
#include "sensorDescriptionStructure.h"
#include "actionDescriptorStructure.h"

extern const char *sqlite_filename;

void prepareSQLDatabase();

void enureAllSensorDescriptionInDB(GList *allActions);

#endif