#include "sqlite_store_sensor_stat.h"

#include <glib.h>
#include <sqlite3.h>
#include <syslog.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "utilityFunctions.h"

static const char *sqlInsertSensorDataStatement = "INSERT INTO sensorStats (sensorID, sensorDisplayName, measurementTime, sensorValue) values (?, ?, ?, ?);";

long sqliteStore_initActionFunction(GHashTable *sensorStatus) {
	struct sqlStatusActionStatus *sql = (struct sqlStatusActionStatus*) malloc(sizeof(sqlStatusActionStatus));
//	prepare_db_file(sqlite_filename);

	if (sqlite3_open(sqlite_filename, &sql->db) != SQLITE_OK) {
		logErrorMessage("Error occured opening SQLite DB file: %s", sqlite3_errmsg(sql->db));
		return -1;
	}

	if (sqlite3_prepare_v2(sql->db, sqlInsertSensorDataStatement, strlen(sqlInsertSensorDataStatement), &sql->preparedStmt, NULL) != SQLITE_OK) {
		logErrorMessage("Error occured compiling prepared statement %s:", sqlite3_errmsg(sql->db));
		return -2;
	}
	
	g_hash_table_replace(sensorStatus, (gpointer) sqliteSensorStatusName, sql);
	return 20000;
}


void sqliteSensorFnc(gpointer key, gpointer value, gpointer sqliteStatus) {
	struct sqlStatusActionStatus *sql = (struct sqlStatusActionStatus *) sqliteStatus;
	struct actionOutputItem *v = (struct actionOutputItem *) value;

	if ((sqlite3_bind_text(sql->preparedStmt, 1, key, strlen((char *)key), SQLITE_STATIC) != SQLITE_OK) ||
		(sqlite3_bind_text(sql->preparedStmt, 2, v->sensorDisplayName, strlen(v->sensorDisplayName), SQLITE_STATIC) != SQLITE_OK) ||
		(sqlite3_bind_int64(sql->preparedStmt, 3, v->timeValueMeasured) != SQLITE_OK) ||
		(sqlite3_bind_double(sql->preparedStmt, 4, v->sensorValue) != SQLITE_OK)) {
			logErrorMessage("Error filling in prepared statement: %s", sqlite3_errmsg(sql->db));
	}

	int ret = sqlite3_step(sql->preparedStmt);
	ret = sqlite3_reset(sql->preparedStmt);
	if (ret != SQLITE_OK) {
		logErrorMessage("Error executing prepared statement: %s", sqlite3_errmsg(sql->db));
	}

	return;
}

long sqliteStore_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus) {
	struct sqlStatusActionStatus *status = (struct sqlStatusActionStatus *)g_hash_table_lookup(sensorStatus, sqliteSensorStatusName);

	g_hash_table_foreach(measurementOutput, &sqliteSensorFnc, (gpointer) status);

	return sqliteSensorInterval;
}

void sqliteStore_closeActionFunction(GHashTable *sensorStatus) {
	struct sqlStatusActionStatus *status = (struct sqlStatusActionStatus *)g_hash_table_lookup(sensorStatus, sqliteSensorStatusName);
	if (status == NULL) {
		syslog(LOG_ERR, "Print to console sensor status was not found. Can not continue.");
		return;
	};
	
	sqlite3_close(status->db);
	free(status);

	g_hash_table_remove(sensorStatus, (gpointer) sqliteSensorStatusName);

}
