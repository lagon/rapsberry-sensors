#include "actionQueue.h"
#include "h21df_action.h"
#include "h21df_library.h"
#include <syslog.h>

const long h21_measurement_offset = 15 * 1000 * 1000; //15 secs

#define __h21dfTemperatureSensorName "H21DF-Temperature"
#define __h21dfHumiditySensorName "H21DF-Humidity"

// const char* h21dfTemperatureSensorName = __h21dfTemperatureSensorName;
// const char* h21dfHumiditySensorName = __h21dfHumiditySensorName;
const char* h21dfSensorStateName = "H21DF";

struct h21dfSensorState_t {
	struct h21dfDevice *device;
	struct allSensorsDescription_t *allSensors;
	char *temperatureSensorName;
	char *humiditySensorName;
	char *sensorStateName;	
};

struct allSensorsDescription_t h21df_allSensors = {
	.numSensors = 2,
	.sensorDescriptions = {{
			.sensorID = __h21dfTemperatureSensorName, 
			.sensorDisplayName = "Temperature (HTU21DF)", 
			.sensorUnits = "C", 
			.sensorValueName = "Temperature"
		}, {
			.sensorID = __h21dfHumiditySensorName, 
			.sensorDisplayName = "Relative Humidity",
			.sensorUnits = "\%", 
			.sensorValueName = "Relative Humidity"
		}
	}
};

struct actionReturnValue_t h21df_returnStructure;

struct actionReturnValue_t* h21df_initActionFunction(char *nameAppendix, char *address) {
	struct h21dfDevice *dev = h21DF_init(1);
	if (dev == NULL) {
		syslog(LOG_ERR, "Unable to initiate h21df humidity sensor.");
		h21df_returnStructure.actionErrorStatus = -1;
		h21df_returnStructure.usecsToNextInvocation = -1;
		h21df_returnStructure.sensorState = NULL;
		return &h21df_returnStructure;
	}

	struct h21dfSensorState_t *state = (struct h21dfSensorState_t *) malloc(sizeof(struct h21dfSensorState_t));
	state->device = dev;
	state->temperatureSensorName = allocateAndConcatStrings(__h21dfTemperatureSensorName, nameAppendix);
	state->temperatureSensorName = allocateAndConcatStrings(__h21dfHumiditySensorName, nameAppendix);
	state->sensorStateName       = allocateAndConcatStrings(h21dfSensorStateName, nameAppendix);
	state->allSensors            = constructAllSensorDescription(&h21df_allSensors, nameAppendix);

	h21df_returnStructure.sensorState = state;
	h21df_returnStructure.actionErrorStatus = 0;
	h21df_returnStructure.usecsToNextInvocation = h21_measurement_offset;
	h21df_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	h21df_returnStructure.changedInputs = generateNoInputsChanged();
	return &h21df_returnStructure;
}

struct actionReturnValue_t* h21df_actionFunction(gpointer rawSensorStatus, GHashTable* measurementOutput, GHashTable *allInputs) {
	struct h21dfSensorState_t *state = (struct h21dfSensorState_t *)rawSensorStatus;

	double temperature = h21DF_readTemperature(state->device);
	double humidity = h21DF_readHumidity(state->device);

	struct actionOutputItem *ao_temperature = (struct actionOutputItem *) malloc(sizeof(actionOutputItem));
	struct actionOutputItem *ao_humidity    = (struct actionOutputItem *) malloc(sizeof(actionOutputItem));

	ao_temperature->sensorDisplayName = state->temperatureSensorName;
	ao_temperature->timeValueMeasured = getCurrentUSecs();
	ao_temperature->sensorValue       = temperature;

	ao_humidity->sensorDisplayName = state->humiditySensorName;
	ao_humidity->timeValueMeasured = getCurrentUSecs();
	ao_humidity->sensorValue       = humidity;

	g_hash_table_replace(measurementOutput, (gpointer) state->temperatureSensorName, ao_temperature);
	g_hash_table_replace(measurementOutput, (gpointer) state->humiditySensorName, ao_humidity);

	h21df_returnStructure.sensorState = state;
	h21df_returnStructure.actionErrorStatus = 0;
	h21df_returnStructure.usecsToNextInvocation = h21_measurement_offset;
	h21df_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	h21df_returnStructure.changedInputs = generateNoInputsChanged();

	return &h21df_returnStructure;
}

const char *h21df_getActionName(gpointer sensorStatus) {
	struct h21dfSensorState_t *state = (struct h21dfSensorState_t *)sensorStatus;
	return state->sensorStateName;
}

struct inputNotifications_t *h21df_actionStateWatchedInputs() {
	return &noInputsToWatch;
}

struct allSensorsDescription_t *h21df_actionStateAllSensors(gpointer sensorStatus) {
	struct h21dfSensorState_t *state = (struct h21dfSensorState_t *)sensorStatus;
	return state->allSensors;
}

void h21df_closeActionFunction(void *sensorStatus) {
	struct h21dfSensorState_t *state = (struct h21dfSensorState_t *)sensorStatus;
	h21DF_close(state->device);
	free(state->allSensors); //NOT SUFFICIENT
	free(state->temperatureSensorName);
	free(state->humiditySensorName);
	free(state->sensorStateName);	
	free(sensorStatus);

	return;
}

struct actionDescriptorStructure_t h21dfActionStructure = {
	.sensorType = "H21DF",
	.sensorStatePtr = NULL,
	.initiateActionFunction = &h21df_initActionFunction,
	.stateWatchedInputs = &h21df_actionStateWatchedInputs,
	.stateAllSensors = &h21df_actionStateAllSensors,
	.actionFunction = &h21df_actionFunction,
	.getActionNameFunction = &h21df_getActionName,
	.destroyActionFunction = &h21df_closeActionFunction
};
