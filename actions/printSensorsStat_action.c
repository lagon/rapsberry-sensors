#include "printSensorsStat_action.h"
#include <stdio.h>
#include <malloc.h>
#include <syslog.h>
#include "sensorDescriptionStructure.h"
#include "actionDescriptorStructure.h"

const char *printSensorStatusName = "PrintOutStatus";
const long printSensorInterval = 15 * 1000 * 1000; //5 secs

struct actionDescriptorStructure_t printActionStructure;

struct actionReturnValue_t print_returnValue;

struct printActionStatus {
	char *sensorStateName;
	long last_printout;
} printActionStatus;

struct allSensorsDescription_t print_allSensors = {
	.numSensors = 0,
	.sensorDescriptions = {}
};

struct actionReturnValue_t *print_initActionFunction(char *nameAppendix, char *address, char *sensorOptions) {
	struct printActionStatus *stat = (struct printActionStatus *)malloc(sizeof(struct printActionStatus));
	stat->last_printout = getCurrentUSecs();
	stat->sensorStateName = allocateAndConcatStrings(printSensorStatusName, nameAppendix);
	print_returnValue.sensorState = stat;
	print_returnValue.actionErrorStatus = 0;
	print_returnValue.usecsToNextInvocation = printSensorInterval;
	print_returnValue.waitOnInputMode = WAIT_TIME_PERIOD;
	print_returnValue.changedInputs = generateNoInputsChanged();

	return &print_returnValue;
}

void printSensorFnc(gpointer key, gpointer value, gpointer userData) {
	struct actionOutputItem *v = (struct actionOutputItem *) value;
	printf("Sensor %s has value of %.1f\n", v->sensorDisplayName, v->sensorValue);
}

struct actionReturnValue_t *print_actionFunction(void *status, GHashTable* measurementOutput, GHashTable *sensorStatus) {
	g_hash_table_foreach(measurementOutput, &printSensorFnc, NULL);
	print_returnValue.changedInputs = generateNoInputsChanged();
	return &print_returnValue;
}

const char *print_getActionName(gpointer sensorStatePtr) {
	struct printActionStatus *sensorState = (struct printActionStatus *)sensorStatePtr;
	return sensorState->sensorStateName;
}

struct inputNotifications_t *print_actionStateWatchedInputs() {
	return &noInputsToWatch;
}

struct allSensorsDescription_t *print_actionStateAllSensors(gpointer ptr) {
	return &print_allSensors;
}


void print_closeActionFunction(void *sensorStatus) {
	free(sensorStatus);
	return;
}


struct actionDescriptorStructure_t printActionStructure = {
	.sensorType = "DisplayPrint",
	.sensorStatePtr = NULL,
	.initiateActionFunction = &print_initActionFunction,
	.stateWatchedInputs = &print_actionStateWatchedInputs,
	.stateAllSensors = &print_actionStateAllSensors,
	.actionFunction = &print_actionFunction,
	.getActionNameFunction = &print_getActionName,
	.destroyActionFunction = &print_closeActionFunction
};