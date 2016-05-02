#include "actionQueue.h"
#include "sfr02_action.h"
#include "sfr02_range.h"
#include <syslog.h>

const long sfr02_range_ranging_offset = 200 * 1000; //1 secs
const long sfr02_range_idle_offset = 800 * 1000; //1 secs

#define __SFR02RangeSensorName "SFR02_Range"

const char* sfr02RangeSensorName = "SFR02";

#define numSamples 60

enum sfr02_rangingState {SFR02_IDLE, SFR02_MEASURING} sfr02_rangingState;

struct sfr02SensorState_t {
	struct sfr02Device *device;
	enum sfr02_rangingState rangingState;
	struct allSensorsDescription_t *allSensors;
	uint16_t distances [numSamples];
	uint8_t firstSampleStartsAt;
	char *rangeSensorName;
	char *sensorStateName;	
	char *sensorName;
};

struct allSensorsDescription_t sfr02_allSensors = {
	.numSensors = 1,
	.sensorDescriptions = {{
			.sensorID = __SFR02RangeSensorName, 
			.sensorDisplayName = "Measured Range (SRF02)", 
			.sensorUnits = "CM", 
			.sensorValueName = "Distance"
		}
	}
};

struct actionReturnValue_t sfr02_returnStructure;

struct actionReturnValue_t* sfr02_initActionFunction(char *nameAppendix, char *address) {
	struct sfr02SensorState_t *sensor = (struct sfr02SensorState_t *) malloc(sizeof(struct sfr02SensorState_t));
	char *x;
	uint8_t addr = strtol(address, &x, 16);
	if (address != x) {
		sensor->device = sfr02_initDevice(1, addr);
	} else {
		printf("SRF02: Was unable to convert %s into address...\n", address);
		sensor->device = NULL;
	}

	if (sensor->device != NULL) {
		sfr02_setMeasurementUnits(sensor->device, SFR02_CM);
		sensor->allSensors = constructAllSensorDescription(&sfr02_allSensors, nameAppendix);
		sensor->sensorStateName = allocateAndConcatStrings(sfr02RangeSensorName, nameAppendix);
		sensor->sensorName = allocateAndConcatStrings(sfr02RangeSensorName, nameAppendix);
		sensor->rangingState = SFR02_IDLE;
		sensor->firstSampleStartsAt = 0;
		memset(sensor->distances, 0, sizeof(uint16_t) * numSamples);

		sfr02_returnStructure.sensorState = sensor;
		sfr02_returnStructure.actionErrorStatus = 0;
		sfr02_returnStructure.usecsToNextInvocation = sfr02_range_idle_offset;
		sfr02_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
		sfr02_returnStructure.changedInputs = generateNoInputsChanged();
	} else {
		sfr02_returnStructure.actionErrorStatus = -1;
	}

	return &sfr02_returnStructure;
}

void recordReading(uint16_t *distances, uint8_t firstSampleStartsAt, uint16_t rangeReading) {
	distances[firstSampleStartsAt] = rangeReading;
}

double getReportedDistance(uint16_t *distances, uint8_t firstSampleStartsAt) {
	double reportedDist = 0.0;
	for (int i = 0; i < numSamples; i++) {
		reportedDist += distances[firstSampleStartsAt];
		firstSampleStartsAt = (firstSampleStartsAt + 1) % numSamples;
	}
	return reportedDist / (double) numSamples;
}

struct actionReturnValue_t* sfr02_actionFunction(gpointer rawSensorStatus, GHashTable* measurementOutput, GHashTable *allInputs) {

	struct sfr02SensorState_t *state = (struct sfr02SensorState_t *)rawSensorStatus;

	sfr02_returnStructure.actionErrorStatus = 0;
	sfr02_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	sfr02_returnStructure.changedInputs = generateNoInputsChanged();

	if (state->rangingState == SFR02_IDLE) {
		sfr02_initiateReading(state->device);
		state->rangingState = SFR02_MEASURING;
		sfr02_returnStructure.usecsToNextInvocation = sfr02_range_ranging_offset;
	} else {
		uint16_t range;
		sfr02_LastReadingValue(state->device, &range);
		printf("Distance reading: %d\n", range);
		
		recordReading(state->distances, state->firstSampleStartsAt, range);
		double reportedRange = getReportedDistance(state->distances, state->firstSampleStartsAt);
		printf("Distance reported: %g cm\n", reportedRange);
		state->firstSampleStartsAt = (state->firstSampleStartsAt + 1) % numSamples;

		state->rangingState = SFR02_IDLE;
		sfr02_returnStructure.usecsToNextInvocation = sfr02_range_idle_offset;

		struct actionOutputItem *ao_range = generateOutputItem(state->sensorName, reportedRange);

		g_hash_table_replace(measurementOutput, (gpointer) state->sensorName, ao_range);
	}

	sfr02_returnStructure.sensorState = state;
	return &sfr02_returnStructure;
}

struct inputNotifications_t *sfr02_actionStateWatchedInputs() {
	return &noInputsToWatch;
}

struct allSensorsDescription_t *sfr02_actionStateAllSensors(gpointer sensorStatus) {
	struct sfr02SensorState_t *state = (struct sfr02SensorState_t *)sensorStatus;
	return state->allSensors;
}

const char *sfr02_getActionName(gpointer sensorStatus) {
	struct sfr02SensorState_t *state = (struct sfr02SensorState_t *)sensorStatus;
	return state->sensorStateName;
}

void sfr02_closeActionFunction(void *sensorStatus) {
	struct sfr02SensorState_t *state = (struct sfr02SensorState_t *)sensorStatus;
	sfr02_closeDevice(state->device);
	free(state->allSensors); //NOT SUFFICIENT
	free(state->rangeSensorName);
	free(state->sensorStateName);
	free(state->sensorName);	
	free(sensorStatus);

	return;
}

struct actionDescriptorStructure_t sfr02ActionStructure = {
	.sensorType = "SFR02",
	.sensorStatePtr = NULL,
	.initiateActionFunction = &sfr02_initActionFunction,
	.actionFunction = &sfr02_actionFunction,
	.stateWatchedInputs = &sfr02_actionStateWatchedInputs,
	.stateAllSensors = &sfr02_actionStateAllSensors,
	.getActionNameFunction = &sfr02_getActionName,
	.destroyActionFunction = &sfr02_closeActionFunction
};
