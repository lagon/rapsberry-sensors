#ifndef __lagon_sqlite_db_utility_functions_h__
#define __lagon_sqlite_db_utility_functions_h__

#include <sqlite3.h>
#include <glib.h>
#include "sensorDescriptionStructure.h"
#include "actionDescriptorStructure.h"

sqlite3 *openDbConnection();
void closeDbConnection(sqlite3 *connection);

void prepareSQLDatabase();

void enureAllSensorDescriptionInDB(GList *allActions);

GList *readExternalInputsFromDb();

#endif