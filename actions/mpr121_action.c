#include <mpr121_action.h>
#include <mpr121.h>
#include "actionQueue.h"
#include "actionDescriptorStructure.h"

struct mpr121_sensorStat {
	struct mpr121_device *device;
	uint8_t keysPressed [13];

	char *sensorStateName;
	char *sensorName;
} ina219_sensorStat;

struct allSensorsDescription_t mpr121_noSensors = {
	.numSensors = 0,
	.sensorDescriptions = NULL
};

struct actionReturnValue_t mpr121_returnStructure;

long long mpr121_touchSensorTimeInterval = 500 * 1000 * 1000;

struct actionReturnValue_t *mpr121_initActionFunction(char *nameAppendix, char *address) {
	struct mpr121_sensorStat *sensor = (struct mpr121_sensorStat *) malloc(sizeof(struct mpr121_sensorStat));
	memset(sensor->keysPressed, 0, 13);

	char *x;
	uint8_t addr = strtol(address, &x, 16);
	if (address != x) {
		sensor->device = mpr121_initializeWithAllElectrodesEnabled(1, addr);
	} else {
		printf("MPR121: Was unable to convert %s into address...\n", address);
		sensor->device = NULL;
	}

	if (sensor->device != NULL) {
		sensor->sensorStateName = allocateAndConcatStrings(mpr121TouchSensorStateName, nameAppendix);
		sensor->sensorName = allocateAndConcatStrings(mpr121TouchSensorName, nameAppendix);

		mpr121_returnStructure.sensorState = sensor;
		mpr121_returnStructure.actionErrorStatus = 0;
		mpr121_returnStructure.usecsToNextInvocation = mpr121_touchSensorTimeInterval;
		mpr121_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
		mpr121_returnStructure.changedInputs = generateNoInputsChanged();

	} else {
		mpr121_returnStructure.actionErrorStatus = -1;		
	}

	return &mpr121_returnStructure;
}

void updateKeysPressed(struct mpr121_device *device, uint8_t keys[13]) {
	for (uint8_t i = 0; i < 13; i++) {
		keys[i] = mpr121_isElectrodeTouched(device, i);
	}
}

struct inputsChanged_t *generateChangedInputs(struct mpr121_sensorStat *mpr121, uint8_t actualKeys[13]) {
	struct inputsChanged_t *x;
	int changedKeys = 0;
	for (int i = 0; i < 13; i++) {
		if ((actualKeys[i] != mpr121->keysPressed[i]) || (actualKeys[i] == 1)) {
			changedKeys++
		}
	}
	if (changedKeys == 0) {
		return generateNoInputsChanged();
	} else {
		struct inputsChanged_t *keysInput = (struct inputsChanged_t*) malloc(sizeof(struct inputsChanged_t) + sizeof(struct inputValue_t) * changedKeys);
		int changedInputId = 0;
		for (int i = 0; i < 13; i++) {
			if ((actualKeys[i] != mpr121->keysPressed[i]) || (actualKeys[i] == 1)) {
				keysInput->newInputValues[changedInputId].inputName = allocateAndConcatStrings(mpr121->sensorName, keyNames[i]);
				keysInput->newInputValues[changedInputId].valueMeasuredTimestamp = getCurrentUSecs();
				keysInput->newInputValues[changedInputId].type = InputTypeInteger;
				keysInput->newInputValues[changedInputId].integerValue = actualKeys[i];
				changedInputId++;
			}
		}
		return keysInput;
	}
}

void replacePressedKeysWithActualValues(struct mpr121_sensorStat *mpr121, uint8_t actualKeys[13]) {
	for (int i = 0; i < 13; i++) {
		mpr121->keysPressed[i] = actualKeys[i];
	}
}

struct actionReturnValue_t* mpr121_actionFunction(gpointer mpr121SensorStat, GHashTable* measurementOutput, GHashTable *sensorStatus) {
	struct mpr121_sensorStat *mpr121 = (struct mpr121_sensorStat *)mpr121SensorStat;

	uint8_t actualKeys[13];
	updateKeysPressed(mpr121, actualKeys);
	struct inputsChanged_t *keysInput = generateChangedInputs(mpr121, actualKeys);
	replacePressedKeysWithActualValues(mpr121, actualKeys);

	mpr121_returnStructure.sensorState = sensor;
	mpr121_returnStructure.usecsToNextInvocation = mpr121_touchSensorTimeInterval;
	mpr121_returnStructure.actionErrorStatus = 0;
	mpr121_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	mpr121_returnStructure.changedInputs = keysInput;

	return &mpr121_returnStructure;
}

struct inputNotifications_t *mpr121_actionStateWatchedInputs() {
	return &noInputsToWatch;
}

struct allSensorsDescription_t* mpr121_actionStateAllSensors(gpointer sensorStatePtr) {
	return &mpr121_noSensors;
}

const char *mpr121_getActionName(gpointer mpr121SensorStat) {
	struct mpr121_sensorStat *mpr121 = (struct mpr121_sensorStat *)mpr121SensorStat;
	return mpr121->sensorStateName;
}

void mpr121_closeActionFunction(gpointer mpr121SensorStat) {
	struct mpr121_sensorStat *mpr121 = (struct mpr121_sensorStat *)mpr121SensorStat;
	mpr121_finishAndClose(mpr121->device);
	free(mpr121->sensorStateName);
	free(mpr121->sensorName);
	free(mpr121);
}

struct actionDescriptorStructure_t mpr121ActionStructure = {
	.sensorType             = "MPR121",
	.sensorStatePtr         = NULL,
	.initiateActionFunction = &mpr121_initActionFunction,
	.stateWatchedInputs     = &mpr121_actionStateWatchedInputs,
	.stateAllSensors        = &mpr121_actionStateAllSensors,
	.actionFunction         = &mpr121_actionFunction,
	.getActionNameFunction  = &mpr121_getActionName,
	.destroyActionFunction  = &mpr121_closeActionFunction
};
