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

static const char *sqliteSensorStatusName = "SQLLiteStoreStatus";
static const long sqliteSensorInterval = 10 * 60 * 1000 * 1000; //10 mins

struct actionReturnValue_t sqliteSave_returnStructure;

struct allSensorsDescription_t sqliteSave_noSensors = {
	.numSensors = 0,
	.sensorDescriptions = {}
};

struct sqlStatusActionStatus {
	sqlite3 *db;
	sqlite3_stmt *preparedStmt;
} sqlStatusActionStatus;

static const char *sqlInsertSensorDataStatement = "INSERT INTO sensorStats (sensorID, sensorDisplayName, measurementTime, sensorValue) values (?, ?, ?, ?);";

struct actionReturnValue_t *sqliteStore_initActionFunction() {
	struct sqlStatusActionStatus *sql = (struct sqlStatusActionStatus*) malloc(sizeof(sqlStatusActionStatus));
//	prepare_db_file(sqlite_filename);

	sqliteSave_returnStructure.sensorState = sql;
	sqliteSave_returnStructure.actionErrorStatus = 0;
	sqliteSave_returnStructure.usecsToNextInvocation = sqliteSensorInterval;
	sqliteSave_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	sqliteSave_returnStructure.changedInputs = &noInputsChanged;

	sql->db = openDbConnection();

	if (sqlite3_prepare_v2(sql->db, sqlInsertSensorDataStatement, strlen(sqlInsertSensorDataStatement), &sql->preparedStmt, NULL) != SQLITE_OK) {
		logErrorMessage("Error occured compiling prepared statement %s:", sqlite3_errmsg(sql->db));
		sqliteSave_returnStructure.actionErrorStatus = 2;
		sqlite3_close(sql->db);
		return &sqliteSave_returnStructure;
	}
	printf("Autocommit: %s\n", sqlite3_get_autocommit(sql->db) == 0 ? "NO" : "YES");
	return &sqliteSave_returnStructure;
}

struct inputNotifications_t *sqliteStore_actionStateWatchedInputs() {
	return &noInputsToWatch;
}

struct allSensorsDescription_t* sqliteStore_actionStateAllSensors() {
	return &sqliteSave_noSensors;
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
		logErrorMessage("Was for sensor: %s", key);
	}
	return;
}

struct actionReturnValue_t *sqliteStore_actionFunction(gpointer rawSensorStatus, GHashTable* measurementOutput, GHashTable *allInputs) {
	struct sqlStatusActionStatus *status = (struct sqlStatusActionStatus *)rawSensorStatus;
	g_hash_table_foreach(measurementOutput, &sqliteSensorFnc, (gpointer) status);

	sqliteSave_returnStructure.sensorState = status;
	sqliteSave_returnStructure.actionErrorStatus = 0;
	sqliteSave_returnStructure.usecsToNextInvocation = sqliteSensorInterval;
	sqliteSave_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	sqliteSave_returnStructure.changedInputs = &noInputsChanged;

	return &sqliteSave_returnStructure;
}

const char *sqliteStore_getActionName() {
	return sqliteSensorStatusName;
}

void sqliteStore_closeActionFunction(gpointer rawSensorStatus) {
	struct sqlStatusActionStatus *status = (struct sqlStatusActionStatus *)rawSensorStatus;
	sqlite3_finalize(status->preparedStmt);
	closeDbConnection(status->db);
}

struct actionDescriptorStructure_t save2SqlActionStructure = {
	.initiateActionFunction = &sqliteStore_initActionFunction,
	.stateWatchedInputs = &sqliteStore_actionStateWatchedInputs,
	.stateAllSensors = &sqliteStore_actionStateAllSensors,
	.actionFunction = &sqliteStore_actionFunction,
	.getActionNameFunction = &sqliteStore_getActionName,
	.destroyActionFunction = &sqliteStore_closeActionFunction
};
