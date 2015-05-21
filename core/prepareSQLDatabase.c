#include "prepareSQLDatabase.h"

#include <sqlite3.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "utilityFunctions.h"
#include "sensorDescriptionStructure.h"

const char *sqlite_filename = "./data/sensor_stats.db";
static const char *sqlCreateSensorNameTable = "create table sensorNames (sensorID varchar not null, sensorDisplayName varchar, sensorUnits varchar, sensorValueName varchar, primary key (sensorID));";
static const char *sqlCreateSensorMeasurementTable = "create table sensorStats (sensorID varchar not null, sensorDisplayName varchar, measurementTime bigint not null, sensorValue double, primary key (sensorID, measurementTime));";
static const char *sqlCreateInputsTable = "create table inputs (inputName varchar not null, stringValue varchar, doubleValue double, primary key (inputName));";


void prepareSQLDatabase() {
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
		logErrorMessage("Error occured opening SQLite DB file: %s \n", sqlite3_errmsg(db));
		_exit(-1);
	}

	sqlite3_exec(db, sqlCreateSensorNameTable, NULL, NULL, NULL);
	sqlite3_exec(db, sqlCreateSensorMeasurementTable, NULL, NULL, NULL);
	sqlite3_exec(db, sqlCreateInputsTable, NULL, NULL, NULL);
	sqlite3_close(db);
}

void ensureSensorInDB(gpointer data, gpointer db) {
	struct singleSensorDescription_t *sensor = (struct singleSensorDescription_t *) data;
	sqlite3 *db3 = (sqlite3 *)db;
	sqlite3_stmt *preparedStmt;

	printf("Checking if sensor %s is in DB\n", sensor->sensorDisplayName);
	if (sqlite3_prepare_v2(db3, "SELECT count(*) as cnt FROM sensorNames where sensorID = ?;", -1, &preparedStmt, NULL) != SQLITE_OK) {
		logErrorMessage("Error occured compiling prepared statement: %s\n", sqlite3_errmsg(db3));
		return;
	};

	if (sqlite3_bind_text(preparedStmt, 1, sensor->sensorID, strlen((char *)sensor->sensorID) * sizeof(char), SQLITE_STATIC) != SQLITE_OK) {
		logErrorMessage("Error occured compiling prepared statement: %s\n", sqlite3_errmsg(db3));
		return;		
	}

	if(sqlite3_step(preparedStmt) != SQLITE_ROW) {
		logErrorMessage("Error occured executing prepared statement: %s\n", sqlite3_errmsg(db3));
		return;				
	}
	int num_records = sqlite3_column_int(preparedStmt, 1);
	if (num_records > 0) {
		//Sensor with given ID is already in DB.
		printf("\tSensor found, skipping...\n");
		return;
	}
	sqlite3_reset(preparedStmt);

	if (sqlite3_prepare_v2(db3, "INSERT INTO sensorNames (sensorID, sensorDisplayName, sensorUnits, sensorValueName) VALUES (?, ?, ?, ?);", -1, &preparedStmt, NULL) != SQLITE_OK) {
		logErrorMessage("Error occured compiling prepared statement:", sqlite3_errmsg(db3));
		return;
	};

	if ((sqlite3_bind_text(preparedStmt, 1, sensor->sensorID,          strlen((char *)sensor->sensorID) * sizeof(char), SQLITE_STATIC) != SQLITE_OK) ||
		(sqlite3_bind_text(preparedStmt, 2, sensor->sensorDisplayName, strlen((char *)sensor->sensorDisplayName) * sizeof(char), SQLITE_STATIC) != SQLITE_OK) ||
		(sqlite3_bind_text(preparedStmt, 3, sensor->sensorUnits,       strlen((char *)sensor->sensorUnits) * sizeof(char), SQLITE_STATIC) != SQLITE_OK) ||
		(sqlite3_bind_text(preparedStmt, 4, sensor->sensorValueName,   strlen((char *)sensor->sensorValueName) * sizeof(char), SQLITE_STATIC) != SQLITE_OK)) {
		logErrorMessage("Error occured compiling prepared statement: %s\n", sqlite3_errmsg(db3));
		return;
	}

	printf("Adding to DB:\n\tID: %s\n\tName: %s\n\tUnits: %s\n\tValue name: %s\n", sensor->sensorID, sensor->sensorDisplayName, sensor->sensorUnits, sensor->sensorValueName);

	sqlite3_step(preparedStmt);
	sqlite3_reset(preparedStmt);

	return;
};


void extractSensorsFromSingleAction(gpointer rawActionPtr, gpointer allSensorsPtr) {
	struct actionDescriptorStructure_t *action = (struct actionDescriptorStructure_t *)rawActionPtr;
	GList *allSensors = *((GList **)allSensorsPtr);
	printf("Examining action %s for inputs. \n", action->getActionNameFunction());
	struct allSensorsDescription_t *sensors = action->stateAllSensors();

	for (int sensID = 0; sensID < sensors->numSensors; sensID++) {
		printf("\tSensor name %s\n", sensors->sensorDescriptions[sensID].sensorDisplayName);
		//allSensors = g_list_append(allSensors, &(sensors->sensorDescriptions[sensID]));
	}
	allSensorsPtr = &allSensors;
}


GList *extractAllSensorsFromActions(GList *allActions) {
	GList *allSensors;

	//g_list_foreach(allActions, &extractSensorsFromSingleAction, &allSensors);
	return(allSensors);
}

///List of singleSensorDescription_t
void enureAllSensorDescriptionInDB(GList *allActions) {
	sqlite3 *db;
	if (sqlite3_open(sqlite_filename, &db) != SQLITE_OK) {
		logErrorMessage("Error occured opening SQLite DB file: %s\n", sqlite3_errmsg(db));
		_exit(-1);
	}

	GList *allSensors = extractAllSensorsFromActions(allActions);
	//g_list_foreach(allSensors, &ensureSensorInDB, (gpointer) db);

	sqlite3_close(db);
};