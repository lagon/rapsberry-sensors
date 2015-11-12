#include "sqliteDbUtilityFunctions.h"

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

const char *sqlCreateSensorNameTable        = "create table sensorNames (sensorID varchar not null, sensorDisplayName varchar, sensorUnits varchar, sensorValueName varchar, primary key (sensorID));";
const char *sqlCreateSensorMeasurementTable = "create table sensorStats (sensorID varchar not null, sensorDisplayName varchar, measurementTime bigint not null, sensorValue double, primary key (sensorID, measurementTime));";
const char *sqlCreateInputsTable            = "create table inputs (inputName varchar not null, stringValue varchar, doubleValue double, primary key (inputName));";

const char *sqlRemoveInputValuesFromDB      = "DELETE FROM inputs;";
const char *sqlReadInputValuesFromDB        = "SELECT inputName, stringValue, doubleValue FROM inputs;";

const char *sqlite_filename = "./data/sensor_stats.db";

sqlite3 *openDbConnection() {
	sqlite3 *db;
	if (sqlite3_open(sqlite_filename, &db) != SQLITE_OK) {
		logErrorMessage("Error occured opening SQLite DB file: %s \n", sqlite3_errmsg(db));
		_exit(-1);
	}	
	printf("Opening SQLite connection\n");
	return db;
};

void closeDbConnection(sqlite3 *connection) {
	printf("Closing SQLite connection\n");
	sqlite3_close(connection);
}

void prepareSQLDatabase() {
	struct stat statStruct;
	if (stat(sqlite_filename, &statStruct) == 0) {
		printf("SQL file found - reusing.\n");
		return;		
	} else {
		if (errno != ENOENT) {
			perror("SQL file not found and following fatal error occured: ");
			_exit(2);
		}
	}

	printf("SQL file not found - creating new one.\n");
	sqlite3 *db = openDbConnection();

	sqlite3_exec(db, sqlCreateSensorNameTable, NULL, NULL, NULL);
	sqlite3_exec(db, sqlCreateSensorMeasurementTable, NULL, NULL, NULL);
	sqlite3_exec(db, sqlCreateInputsTable, NULL, NULL, NULL);
	closeDbConnection(db);
}

void ensureSensorInDB(gpointer data, gpointer db) {
	if (data == NULL) {
		return;
	}
	struct singleSensorDescription_t *sensor = (struct singleSensorDescription_t *) data;
	sqlite3 *db3 = (sqlite3 *)db;
	sqlite3_stmt *preparedStmt;

	printf("Checking if sensor %s is in DB\n", sensor->sensorDisplayName);
	if (sqlite3_prepare_v2(db3, "SELECT count(*) as cnt FROM sensorNames where sensorID = ?;", -1, &preparedStmt, NULL) != SQLITE_OK) {
		logErrorMessage("Error occured compiling prepared statement: %s\n", sqlite3_errmsg(db3));
		return;
	};

	if (sqlite3_bind_text(preparedStmt, 1, sensor->sensorID, -1, SQLITE_STATIC) != SQLITE_OK) {
		logErrorMessage("Error occured compiling prepared statement: %s\n", sqlite3_errmsg(db3));
		return;		
	}

	if(sqlite3_step(preparedStmt) != SQLITE_ROW) {
		logErrorMessage("Error occured executing prepared statement: %s\n", sqlite3_errmsg(db3));
		return;				
	}
	double num_records = sqlite3_column_double(preparedStmt, 0);
	printf("#Records: %f\n", num_records);

	sqlite3_reset(preparedStmt);
	sqlite3_finalize(preparedStmt);

	if (num_records > 0) {
		//Sensor with given ID is already in DB.
		printf("\tSensor found, skipping...\n");
		return;
	}

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
	sqlite3_finalize(preparedStmt);
	char *errmgs;
	sqlite3_exec(db3, "COMMIT", NULL, NULL, &errmgs);
	if (errmgs != NULL) {
		printf("Error is %s\n", errmgs);
		sqlite3_free(errmgs);
	} else {
		printf("Sensor add commited correctly\n");
	}
	
	return;
};


void extractSensorsFromSingleAction(gpointer rawActionPtr, gpointer allSensorsPtr) {
	struct actionDescriptorStructure_t *action = (struct actionDescriptorStructure_t *)rawActionPtr;
	GList *allSensors = (GList *)allSensorsPtr;
	printf("Examining action %s for inputs. \n", action->getActionNameFunction(action->sensorStatePtr));
	struct allSensorsDescription_t *sensors = action->stateAllSensors(action->sensorStatePtr);

	for (int sensID = 0; sensID < sensors->numSensors; sensID++) {
	 	printf("\tSensor name %s\n", sensors->sensorDescriptions[sensID].sensorDisplayName);
	 	allSensors = g_list_append(allSensors, &(sensors->sensorDescriptions[sensID]));
	}
}


GList *extractAllSensorsFromActions(GList *allActions) {
	GList *allSensors = g_list_append(NULL, NULL);
	g_list_foreach(allActions, &extractSensorsFromSingleAction, allSensors);
//`	printf("%d sensors collected\n", g_list_length(allSensors));
	return(allSensors);
}

///List of singleSensorDescription_t
void enureAllSensorDescriptionInDB(GList *allActions) {
	printf("Checking if all sensors are in DB.\n");
	sqlite3 *db = openDbConnection();

	GList *allSensors = extractAllSensorsFromActions(allActions);
	g_list_foreach(allSensors, &ensureSensorInDB, (gpointer) db);
	g_list_free(allSensors);

	closeDbConnection(db);
	printf("Checking done\n");

};

GList *readExternalInputsFromDb() {
	sqlite3 *db = openDbConnection();
	sqlite3_stmt *preparedStmt;

	if (sqlite3_prepare_v2(db, sqlReadInputValuesFromDB, -1, &preparedStmt, NULL) != SQLITE_OK) {
		logErrorMessage("Error occured compiling prepared statement: %s\n", sqlite3_errmsg(db));
		sqlite3_exec(db, sqlRemoveInputValuesFromDB	, NULL, NULL, NULL);
		closeDbConnection(db);
		return NULL;
	}

	GList *inputs = NULL;
	while(sqlite3_step(preparedStmt) == SQLITE_ROW) {
		struct inputValue_t *inputValue = (struct inputValue_t *) malloc(sizeof(struct  inputValue_t));
		char *input_name = sqlite3_column_text(preparedStmt, 0);

		if ((sqlite3_column_type(preparedStmt, 1) != SQLITE_NULL) && (sqlite3_column_type(preparedStmt, 2) != SQLITE_NULL)) {
			logErrorMessage("Input %s has both string and double value, do not know what to do....\n", input_name);
			free(inputValue);
			continue;
		} else if (sqlite3_column_type(preparedStmt, 1) != SQLITE_NULL) {
			inputValue->type = InputTypeString;
			char * string_value = sqlite3_column_text(preparedStmt, 1);
			inputValue->stringValue = (char *) malloc((strlen(string_value) + 1) * sizeof(char));
			strcpy(inputValue->stringValue, string_value);
			printf("Reading input \'%s\' with value \'%s\'\n", input_name, inputValue->stringValue);
		} else if (sqlite3_column_type(preparedStmt, 2) != SQLITE_NULL) {
			inputValue->type = InputTypeDouble;
			inputValue->doubleValue = sqlite3_column_double(preparedStmt, 2);
			printf("Reading input \'%s\' with value \'%f\'\n", input_name, inputValue->doubleValue);
		} else {
			logErrorMessage("Input %s has no data attached to it.\n", input_name);
			free(inputValue);
			continue;
		}

		inputValue->inputName = (char *)malloc(sizeof(char) * (strlen(input_name)+1));
		strcpy(inputValue->inputName, input_name);
		inputValue->valueMeasuredTimestamp = getCurrentUSecs();
		
		inputs = g_list_append(inputs, inputValue);
	}

	sqlite3_exec(db, sqlRemoveInputValuesFromDB, NULL, NULL, NULL);
	closeDbConnection(db);
	return(inputs);
}
