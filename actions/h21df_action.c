#include "actionQueue.h"
#include "h21df_action.h"
#include "h21df_library.h"
#include <syslog.h>

const long h21_measurement_offset = 15 * 1000 * 1000; //15 secs

#define __h21dfTemperatureSensorName "H21DF-Temperature"
#define __h21dfHumiditySensorName "H21DF-Humidity"

const char* h21dfTemperatureSensorName = __h21dfTemperatureSensorName;
const char* h21dfHumiditySensorName = __h21dfHumiditySensorName;
const char* h21dfSensorStateName = "H21DF";

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

struct actionReturnValue_t* h21df_initActionFunction() {

	struct h21dfDevice *dev = h21DF_init(1);
	if (dev != NULL) {
		h21df_returnStructure.sensorState = dev;
		h21df_returnStructure.actionErrorStatus = 0;
		h21df_returnStructure.usecsToNextInvocation = h21_measurement_offset;
		h21df_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
		h21df_returnStructure.changedInputs = generateNoInputsChanged();
		return &h21df_returnStructure;
	} else {
		syslog(LOG_ERR, "Unable to initiate h21df humidity sensor.");
		h21df_returnStructure.actionErrorStatus = -1;
		h21df_returnStructure.usecsToNextInvocation = -1;
		return &h21df_returnStructure;
	}
}

struct actionReturnValue_t* h21df_actionFunction(gpointer rawSensorStatus, GHashTable* measurementOutput, GHashTable *allInputs) {
	struct h21dfDevice *dev = (struct h21dfDevice *)rawSensorStatus;

	if (dev == NULL) {
		syslog(LOG_ERR, "H21DF sensor status was not found. Can not continue.");
		h21df_returnStructure.sensorState = dev;
		h21df_returnStructure.actionErrorStatus = -1;
		h21df_returnStructure.usecsToNextInvocation = -1;
		h21df_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
		h21df_returnStructure.changedInputs = generateNoInputsChanged();
		return &h21df_returnStructure;
	};

	double temperature = h21DF_readTemperature(dev);
	double humidity = h21DF_readHumidity(dev);

	struct actionOutputItem *ao_temperature = (struct actionOutputItem *) malloc(sizeof(actionOutputItem));
	struct actionOutputItem *ao_humidity    = (struct actionOutputItem *) malloc(sizeof(actionOutputItem));

	ao_temperature->sensorDisplayName = h21dfTemperatureSensorName;
	ao_temperature->timeValueMeasured = getCurrentUSecs();
	ao_temperature->sensorValue       = temperature;

	ao_humidity->sensorDisplayName = h21dfHumiditySensorName;
	ao_humidity->timeValueMeasured = getCurrentUSecs();
	ao_humidity->sensorValue       = humidity;

	g_hash_table_replace(measurementOutput, (gpointer) h21dfTemperatureSensorName, ao_temperature);
	g_hash_table_replace(measurementOutput, (gpointer) h21dfHumiditySensorName, ao_humidity);

	h21df_returnStructure.sensorState = dev;
	h21df_returnStructure.actionErrorStatus = 0;
	h21df_returnStructure.usecsToNextInvocation = h21_measurement_offset;
	h21df_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	h21df_returnStructure.changedInputs = generateNoInputsChanged();

	return &h21df_returnStructure;
}

const char *h21df_getActionName() {
	return h21dfSensorStateName;
}

struct inputNotifications_t *h21df_actionStateWatchedInputs() {
	return &noInputsToWatch;
}

struct allSensorsDescription_t *h21df_actionStateAllSensors() {
	return &h21df_allSensors;
}

void h21df_closeActionFunction(void *sensorStatusPtr) {
	struct h21dfDevice *dev = (struct h21dfDevice *)sensorStatusPtr;

	if (dev == NULL) {
		syslog(LOG_ERR, "H21DF sensor status was not found. Can not continue.");
		return;
	};
	h21DF_close(dev);

	return;
}

struct actionDescriptorStructure_t h21dfActionStructure = {
	.initiateActionFunction = &h21df_initActionFunction,
	.stateWatchedInputs = &h21df_actionStateWatchedInputs,
	.stateAllSensors = &h21df_actionStateAllSensors,
	.actionFunction = &h21df_actionFunction,
	.getActionNameFunction = &h21df_getActionName,
	.destroyActionFunction = &h21df_closeActionFunction
};
