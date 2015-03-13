#include "actionQueue.h"

#include <glib.h>
#include <sqlite3.h>

#ifndef __lagon_sqlite_store_sensor_stat_h__
#define __lagon_sqlite_store_sensor_stat_h__

static const char *sqliteSensorStatusName = "SQLLiteStoreStatus";
static const long sqliteSensorInterval = 15 * 60 * 1000 * 1000; //30 minutes

struct sqlStatusActionStatus {
	sqlite3 *db;
	sqlite3_stmt *preparedStmt;
} sqlStatusActionStatus;

long sqliteStore_initActionFunction(GHashTable *sensorStatus);
long sqliteStore_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus);
void sqliteStore_closeActionFunction(GHashTable *sensorStatus);


#endif