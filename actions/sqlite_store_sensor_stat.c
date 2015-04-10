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

static const char *sqlInsertSensorDataStatement = "INSERT INTO sensorStats (sensorID, sensorDisplayName, measurementTime, sensorValue) values (?, ?, ?, ?);";
static const char *sqlite_filename = "./data/sensor_stats.db";
static const char *sqlCreateSensorNameTable = "create table sensorNames (sensorID varchar not null, sensorDisplayName varchar, sensorUnits varchar, sensorValueName varchar, primary key (sensorID));";
static const char *sqlCreateSensorMeasurementTable = "create table sensorStats (sensorID varchar not null, sensorDisplayName varchar, measurementTime bigint not null, sensorValue double, primary key (sensorID, measurementTime));";
static const char *sqlCreateInputsTable = "create table inputs (inputName varchar not null, stringValue varchar, doubleValue double, primary key (inputName));";

void sqlite_log_error(const char *msg, const char *db_msg) {
		int len =  (strlen(msg) + strlen(db_msg) + 50);
		char * sys_msg = (char *) malloc(sizeof(char) * len);
		snprintf(sys_msg, len, "%s %s", msg, db_msg);
		syslog(LOG_ERR, sys_msg);
		free(sys_msg);
}

void prepare_db_file(const char *filename) {
	int f = open(sqlite_filename, O_RDONLY);
	if (f >= 0) {
		printf("SQL file found - reusing.\n");
		close(f);
		return;
	}

	if (errno != 2) {
		perror("Something very wrong had happened.");
		_exit(-1);
	}

	printf("SQL file not found - creating new one.\n");
	sqlite3 *db;
	if (sqlite3_open(sqlite_filename, &db) != SQLITE_OK) {
		sqlite_log_error("Error occured opening SQLite DB file:", sqlite3_errmsg(db));
		_exit(-1);
	}

	sqlite3_exec(db, sqlCreateSensorNameTable, NULL, NULL, NULL);
	sqlite3_exec(db, sqlCreateSensorMeasurementTable, NULL, NULL, NULL);
	sqlite3_exec(db, sqlCreateInputsTable, NULL, NULL, NULL);
	sqlite3_close(db);
}

long sqliteStore_initActionFunction(GHashTable *sensorStatus) {
	struct sqlStatusActionStatus *sql = (struct sqlStatusActionStatus*) malloc(sizeof(sqlStatusActionStatus));
	prepare_db_file(sqlite_filename);

	if (sqlite3_open(sqlite_filename, &sql->db) != SQLITE_OK) {
		sqlite_log_error("Error occured opening SQLite DB file:", sqlite3_errmsg(sql->db));
		return -1;
	}

	if (sqlite3_prepare_v2(sql->db, sqlInsertSensorDataStatement, strlen(sqlInsertSensorDataStatement), &sql->preparedStmt, NULL) != SQLITE_OK) {
		sqlite_log_error("Error occured compiling prepared statement:", sqlite3_errmsg(sql->db));
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
			sqlite_log_error("Error filling in prepared statement:", sqlite3_errmsg(sql->db));
	}

	int ret = sqlite3_step(sql->preparedStmt);
	ret = sqlite3_reset(sql->preparedStmt);
	if (ret != SQLITE_OK) {
		sqlite_log_error("Error executing prepared statement:", sqlite3_errmsg(sql->db));
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
