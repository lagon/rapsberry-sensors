#include "actionQueue.h"
#include "bmp183_action.h"
#include "bmp183_library.h"
#include <syslog.h>
#include "actionDescriptorStructure.h"
#include "sensorDescriptionStructure.h"
#include "utilityFunctions.h"

const char* bmp183PressureSensorName = "BMP183-Pressure";
const char* bmp183PressureSensorStateName = "BMP183";
const long long bmp183_PressureSensorRefresh = 15 * 1000 * 1000; //15 secs

enum bmp183_measurementState {BMP183_NOP, BMP183_MEASURING_TEMP, BMP183_MEASURING_PRESSURE} bmp183_measurementState;

struct bmp183_sensorStat {
	struct bmp183_device *device;
	enum bmp183_measurementState state;
	char *sensorName;
	char *sensorStateName;
	struct allSensorsDescription_t *allSensors;
} bmp183_sensorStat;

struct allSensorsDescription_t bmp183_allSensors = {
	.numSensors = 1,
	.sensorDescriptions = {{
		.sensorID = "BMP183", 
		.sensorDisplayName = "Athmospheric Pressure", 
		.sensorUnits = "kPa", 
		.sensorValueName = "Pressure"
	}}
};

struct actionReturnValue_t bmp183_returnStructure;

struct actionReturnValue_t* bmp183_initActionFunction(char *nameAppendix, char *address) {
	bmp183_returnStructure.usecsToNextInvocation = bmp183_PressureSensorRefresh;
	bmp183_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	bmp183_returnStructure.changedInputs = generateNoInputsChanged();

	int busID = strtol(address, NULL, 10);
	if ((busID != 0) && (busID != 1)) {
		syslog(LOG_ERR, "BMP183 - Address has to be 0 or 1");
		bmp183_returnStructure.sensorState = NULL;
		bmp183_returnStructure.actionErrorStatus = -1;

		return &bmp183_returnStructure;		
	}

	struct bmp183_sensorStat *status = (struct bmp183_sensorStat *) malloc(sizeof(struct bmp183_device));
	status->device = bmp183_init(busID, 0, 1000000, BMP183_ULTRA_HIGH_RESOLUTION);
	if (status->device == NULL) {
		syslog(LOG_ERR, "Initiation of BMP183 pressure sensor failed.");
		printf("FAILED!");
		free(status);
		
		bmp183_returnStructure.sensorState = NULL;
		bmp183_returnStructure.actionErrorStatus = -1;

		return &bmp183_returnStructure;
	}
	status->state           = BMP183_NOP;
	status->sensorName      = allocateAndConcatStrings(bmp183PressureSensorName, nameAppendix);
	status->sensorStateName = allocateAndConcatStrings(bmp183PressureSensorStateName, nameAppendix);
	status->allSensors      = constructAllSensorDescription(bmp183_allSensors, nameAppendix);
	
	bmp183_returnStructure.sensorState = status;
	bmp183_returnStructure.actionErrorStatus = 0;

	return &bmp183_returnStructure;
}

struct actionReturnValue_t* bmp183_actionFunction(gpointer rawSensorStatus, GHashTable* measurementOutput, GHashTable *allInputs) {
	struct bmp183_sensorStat *status = (struct bmp183_sensorStat *)rawSensorStatus;

	switch (status->state) {
		case BMP183_NOP:
			status->state = BMP183_MEASURING_TEMP;
			bmp183_initiateTemeratureMeasurement(status->device);
			break;
		case BMP183_MEASURING_TEMP:
			status->state = BMP183_MEASURING_PRESSURE;
			double temp = bmp183_readTemperature(status->device);
			printf("Pressure sensor temperature %.2fC\n", temp);
			bmp183_initiatePressureMeasurement(status->device);
			break;
		case BMP183_MEASURING_PRESSURE:
			status->state = BMP183_NOP;
			double pressure = bmp183_readPressure(status->device);
			printf("Current Athmospheric Pressure %.2fPa\n", pressure);

			struct actionOutputItem *ao_pressure = (struct actionOutputItem *)malloc(sizeof(struct actionOutputItem));
			ao_pressure->sensorDisplayName = status->sensorName;
			ao_pressure->timeValueMeasured = getCurrentUSecs();
			ao_pressure->sensorValue = pressure;

			g_hash_table_replace(measurementOutput, (gpointer) status->sensorName, ao_pressure);
			break;
	}

	long timeToNextInvocation = 10000;

	switch (status->state) {
		case BMP183_NOP:
			timeToNextInvocation = bmp183_PressureSensorRefresh;
			break;
		case BMP183_MEASURING_TEMP:
			timeToNextInvocation = bmp183_getMeasurementUSecs(status->device, BMP183_MEASURE_TEMPERATURE);
			break;
		case BMP183_MEASURING_PRESSURE:
			timeToNextInvocation = bmp183_getMeasurementUSecs(status->device, BMP183_MEASURE_PRESSURE);
			break;
	}

	bmp183_returnStructure.sensorState = status;
	bmp183_returnStructure.actionErrorStatus = 0;
	bmp183_returnStructure.usecsToNextInvocation = timeToNextInvocation;
	bmp183_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	bmp183_returnStructure.changedInputs = generateNoInputsChanged();

	return &bmp183_returnStructure;
}

void bmp183_closeActionFunction(gpointer sensorStatus) {
	struct bmp183_sensorStat *status = (struct bmp183_sensorStat *)sensorStatus;
	bmp183_close(status->device);
	free(status->sensorName);
	free(status->sensorStateName);
}

const char *bmp183_getActionName(gpointer sensorStatus) {
	struct bmp183_sensorStat *status = (struct bmp183_sensorStat *)sensorStatus;
	return sensorStatus->sensorStateName;
}

struct inputNotifications_t *bmp183_actionStateWatchedInputs(gpointer sensorStatus) {
	return &noInputsToWatch;
}

struct allSensorsDescription_t *bmp183_actionStateAllSensors(gpointer sensorStatus) {
	struct bmp183_sensorStat *status = (struct bmp183_sensorStat *)sensorStatus;
	return sensorStatus->allSensors;
}


struct actionDescriptorStructure_t bmp183ActionStructure = {
	.sensorType = "BMP183",
	.sensorStatePtr = NULL,
	.initiateActionFunction = &bmp183_initActionFunction,
	.stateWatchedInputs = &bmp183_actionStateWatchedInputs,
	.stateAllSensors = &bmp183_actionStateAllSensors,
	.actionFunction = &bmp183_actionFunction,
	.getActionNameFunction = &bmp183_getActionName,
	.destroyActionFunction = &bmp183_closeActionFunction
};
