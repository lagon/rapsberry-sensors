#include <mpr121_action.h>
#include <mpr121.h>
#include <stdlib.h>
#include "actionQueue.h"
#include "actionDescriptorStructure.h"

 const char *mpr121_keyNames[] = {__MPR_Key1,  __MPR_Key2,  __MPR_Key3,
						  		 __MPR_Key4,  __MPR_Key5,  __MPR_Key6,
						    	 __MPR_Key7,  __MPR_Key8,  __MPR_Key9,
						  		 __MPR_Key10, __MPR_Key11, __MPR_Key12,
						  		 __MPR_KeyProximity};

struct mpr121_sensorStat {
	struct mpr121_device *device;
	uint8_t keysPressed [13];

	char *sensorStateName;
	char *sensorName;
} ina219_sensorStat;

struct allSensorsDescription_t mpr121_noSensors = {
	.numSensors = 0,
	.sensorDescriptions = {}
};

const char* mpr121TouchSensorName = "MPR121";
const char* mpr121TouchSensorStateName = "MPR121";

struct actionReturnValue_t mpr121_returnStructure;

long long mpr121_touchSensorTimeInterval = 100 * 1000;

struct actionReturnValue_t *mpr121_initActionFunction(char *nameAppendix, char *address, char *sensorOptions) {
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
	mpr121_isElectrodeTouched(device, 0);
	while (device->isRunningMode != 1) {
        printf("mpr121 in stop mode. Enabling...");
        //mpr121_resetAndSetup(dev);
        if (mpr121_putToRunningMode(device) > 0) {
            printf("up & running\n");
        } else {
	        printf("down and stop for some reason\n");
        }
        mpr121_isElectrodeTouched(device, 0);
    }
	for (uint8_t i = 0; i < 13; i++) {
		keys[i] = mpr121_isElectrodeTouched(device, i);
		// printf("%c", keys[i] == 0 ? ' ' : 'X');
	}
	// printf("\n");
}

struct inputsChanged_t *generateChangedInputs(struct mpr121_sensorStat *mpr121, uint8_t actualKeys[13]) {
	int changedKeys = 0;
	for (int i = 0; i < 13; i++) {
		if ((actualKeys[i] != mpr121->keysPressed[i])) {
			changedKeys++;
		}
	}
	// printf("Number of keys changed %d\n", changedKeys);
	if (changedKeys == 0) {
		return generateNoInputsChanged();
	} else {
		struct inputsChanged_t *keysInput = (struct inputsChanged_t*) malloc(sizeof(struct inputsChanged_t) + sizeof(struct inputValue_t) * changedKeys);
		keysInput->numInputsChanged = changedKeys;
		int changedInputId = 0;
		for (int i = 0; i < 13; i++) {
			if ((actualKeys[i] != mpr121->keysPressed[i])) {
				keysInput->newInputValues[changedInputId].inputName = allocateAndConcatStrings(mpr121->sensorName, mpr121_keyNames[i]);
				keysInput->newInputValues[changedInputId].valueMeasuredTimestamp = getCurrentUSecs();
				keysInput->newInputValues[changedInputId].type = InputTypeInteger;
				keysInput->newInputValues[changedInputId].integerValue = actualKeys[i];
				// printf("**** %s\n", allocateAndConcatStrings(mpr121->sensorName, mpr121_keyNames[i]));
				printf("Touch %d (%s) - changed to state %d\n", i, keysInput->newInputValues[changedInputId].inputName, keysInput->newInputValues[changedInputId].integerValue);
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
	// printf("Updating keys\n");
	updateKeysPressed(mpr121->device, actualKeys);
	// printf("Generating changed inputs (if any)\n");
	struct inputsChanged_t *keysInput = generateChangedInputs(mpr121, actualKeys);
	// printf("Storing current keys\n");
	replacePressedKeysWithActualValues(mpr121, actualKeys);
	// printf("Generating return structure\n");

	mpr121_returnStructure.sensorState = mpr121;
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
