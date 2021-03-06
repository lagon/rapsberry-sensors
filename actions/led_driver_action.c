#include "led_driver_action.h"
#include "led_driver_library.h"
#include "kbInput_action.h"
#include "utilityFunctions.h"
#include <glib.h>
#include "pattern_queue.h"
#include "all_led_patterns.h"
#include <stdarg.h>
#include "mpr121_action.h"

const char *ledDriverActionName = "LedDriveAction";
const int numLeds = 36;

struct allSensorsDescription_t ledDriver_allSensors = {
	.numSensors = 0,
	.sensorDescriptions = {}
};

struct actionReturnValue_t ledDriver_returnStructure;

#define __APPEND_TOUCH_SENSOR_NAME(SENSOR_NAME, KEY_NAME) SENSOR_NAME KEY_NAME
#define __TOUCH_SENSOR_NAME "MPR121On Cupboard"

struct inputNotifications_t ledDrive_watchedInputs = {
	.numInputsWatched = 6,
	.watchedInputs = {"chodba_webInput", 
						__keyboardInputName, 
						__APPEND_TOUCH_SENSOR_NAME(__TOUCH_SENSOR_NAME, __MPR_Key1), 
						__APPEND_TOUCH_SENSOR_NAME(__TOUCH_SENSOR_NAME, __MPR_Key2), 
						__APPEND_TOUCH_SENSOR_NAME(__TOUCH_SENSOR_NAME, __MPR_Key3), 
						__APPEND_TOUCH_SENSOR_NAME(__TOUCH_SENSOR_NAME, __MPR_Key4)}
};

struct ledDriver_sensorStat {
	int spiDevice;
	struct allLedControlStruct *ledctrl;
	long long lastKBInputUpdate;
	long long lastWebUpdate;
	long long lastTouchSensorUpdate;
	uint16_t currentBrightness;
	struct pa_Queue *ledPatternsQueue;
	char *sensorStateName;


} ledDriver_sensorStat;

void setReturnStructure(struct actionReturnValue_t *returnStructure, struct ledDriver_sensorStat *sensorState);

void setLedsAccordingToPattern(struct allLedControlStruct *ledctrl, struct pa_LedStatesResults *pattern) {
	for (int ledID = 0; ledID < pattern->totalLeds; ledID++) {
//		printf(" ** %X", pattern->ledIntensities[ledID]);
		setOneGrayscaleLed(ledctrl, ledID, pattern->ledIntensities[ledID]);
	}
//	printf("\n");
}

struct actionReturnValue_t* ledDriver_initActionFunction(char *nameAppendix, char *address, char *sensorOptions) {
	printf("Watching for inputs:\n");
	for (int i = 0; i < ledDrive_watchedInputs.numInputsWatched; i++) {
		printf("\t * %s\n", ledDrive_watchedInputs.watchedInputs[i]);
	}

	int spidev = spi_initDevice(0, 0);
	if (spidev < 0) {
		perror("");
		exit(-1);
	}

	struct allLedControlStruct *ledctrl = initiateLEDControls(12);
	setGlobalBrightness(ledctrl, 255);
	struct ledPWMSettings led_settings;
	led_settings.blank  = 0;
	led_settings.dsprpt = 1;
	led_settings.tmgrst = 1;
	led_settings.extgck = 0;
	led_settings.outtmg = 1;
	setLedSettings(ledctrl, &led_settings);

	struct ledDriver_sensorStat *led = (struct ledDriver_sensorStat *)malloc(sizeof(struct ledDriver_sensorStat));
	led->spiDevice = spidev;
	led->ledctrl = ledctrl;
	led->lastKBInputUpdate = 0;
	led->currentBrightness = 0;
	led->ledPatternsQueue = pa_initialize(numLeds);
	led->sensorStateName = allocateAndConcatStrings(ledDriverActionName, nameAppendix);

	setReturnStructure(&ledDriver_returnStructure, led);
	return &ledDriver_returnStructure;
}

void setReturnStructure(struct actionReturnValue_t *returnStructure, struct ledDriver_sensorStat *sensorState) {
	returnStructure->sensorState = (gpointer) sensorState;
	returnStructure->actionErrorStatus = 0;
	returnStructure->usecsToNextInvocation = 500 * 1000;
	returnStructure->waitOnInputMode = WAIT_ON_INPUT;
	returnStructure->changedInputs = generateNoInputsChanged();
	return;
}

void setReturnStructureWithPatterns(struct actionReturnValue_t *returnStructure, struct ledDriver_sensorStat *sensorState, long long nextExpectedPatternStepTime) {
	returnStructure->sensorState = (gpointer) sensorState;
	returnStructure->actionErrorStatus = 0;
	returnStructure->usecsToNextInvocation = nextExpectedPatternStepTime;
	if (nextExpectedPatternStepTime > 0) {
		returnStructure->waitOnInputMode = WAIT_TIME_PERIOD_OR_INPUT;
	} else {
		returnStructure->waitOnInputMode = WAIT_ON_INPUT;		
	}
	returnStructure->changedInputs = generateNoInputsChanged();
	return;	
}


struct inputNotifications_t* ledDriver_actionStateWatchedInputs() {
	return &ledDrive_watchedInputs;
}

char *checkNewStrInput(char *inputName, long long lastSeenUpdate, GHashTable* allInputs) {
	struct inputValue_t *input = (struct inputValue_t *)g_hash_table_lookup(allInputs, inputName);
	if (input == NULL) {
		return NULL;
	}
	if (input->valueMeasuredTimestamp <= lastSeenUpdate) {
		return NULL;
	}
	return input->stringValue;
}

int *checkNewIntInput(char *inputName, long long lastSeenUpdate, GHashTable* allInputs) {
	struct inputValue_t *input = (struct inputValue_t *)g_hash_table_lookup(allInputs, inputName);
	if (input == NULL) {
		return NULL;
	}
	if (input->valueMeasuredTimestamp <= lastSeenUpdate) {
		return NULL;
	}
	return &(input->integerValue);
}

void setupLedPatternsToQueue(struct ledDriver_sensorStat *state, uint16_t initialBrightness, uint16_t targetBrightness, int numPatternsToAdd, ...) {
	va_list valist;
	pa_resetQueue(state->ledPatternsQueue);
	va_start(valist, numPatternsToAdd);
	for (int patternID = 0; patternID < numPatternsToAdd; patternID++) {
		patternAction_t pattern = va_arg(valist, patternAction_t);
		pa_addNextLedAction(state->ledPatternsQueue, pattern, initialBrightness, targetBrightness);
	}
	va_end(valist);
}

long long executePatternStep(struct ledDriver_sensorStat *state) {
	if (pa_isQueueEmpty(state->ledPatternsQueue)) {
		return pa_neverCallAgain;
	}

	struct pa_LedStatesResults *patternStep = pa_executeCurrentLedAction(state->ledPatternsQueue);

	long long timeToNextInvodation = patternStep->nextInvocation;
	if (timeToNextInvodation == pa_wasLastStep) {
		pa_getToNextLedAction(state->ledPatternsQueue);
		timeToNextInvodation = 500 * 1000;
	} else if (timeToNextInvodation == pa_neverCallAgain) {
		pa_resetQueue(state->ledPatternsQueue);
	} else if (timeToNextInvodation == pa_emptyActionQueue) {
		pa_destroyLedStateResults(patternStep);
		return -1;
	}
	setLedsAccordingToPattern(state->ledctrl, patternStep);
	sendOutLedDataDefaults(state->ledctrl, state->spiDevice);

	state->currentBrightness = patternStep->ledIntensities[0];
	pa_destroyLedStateResults(patternStep);
	return timeToNextInvodation;
}

struct actionReturnValue_t* ledDriver_actionFunction(gpointer rawSensorState, GHashTable* measurementOutput, GHashTable* allInputs) {
	struct ledDriver_sensorStat *state = (struct ledDriver_sensorStat *) rawSensorState;

	char *keyboardCmd = checkNewStrInput(__keyboardInputName, state->lastKBInputUpdate, allInputs);
	char *webCmd      = checkNewStrInput("chodba_webInput",   state->lastWebUpdate, allInputs);
	int *touchKey1   = checkNewIntInput(__APPEND_TOUCH_SENSOR_NAME(__TOUCH_SENSOR_NAME, __MPR_Key1), state->lastTouchSensorUpdate, allInputs);
	int *touchKey2   = checkNewIntInput(__APPEND_TOUCH_SENSOR_NAME(__TOUCH_SENSOR_NAME, __MPR_Key2), state->lastTouchSensorUpdate, allInputs);
	int *touchKey3   = checkNewIntInput(__APPEND_TOUCH_SENSOR_NAME(__TOUCH_SENSOR_NAME, __MPR_Key3), state->lastTouchSensorUpdate, allInputs);
	int *touchKey4   = checkNewIntInput(__APPEND_TOUCH_SENSOR_NAME(__TOUCH_SENSOR_NAME, __MPR_Key4), state->lastTouchSensorUpdate, allInputs);

	uint16_t nextBrightness;

	if ((keyboardCmd == NULL) && (webCmd == NULL) 
		&& (touchKey1 == NULL) && (touchKey2 == NULL)
		&& (touchKey3 == NULL) && (touchKey4 == NULL)) {
		if (pa_isQueueEmpty(state->ledPatternsQueue)) {
			setReturnStructure(&ledDriver_returnStructure, state);
		}  else {
			long long nextExpectedPatternStepTime = executePatternStep(state);
			setReturnStructureWithPatterns(&ledDriver_returnStructure, state, nextExpectedPatternStepTime);
		}
		return &ledDriver_returnStructure;	
	}
	
	if (keyboardCmd != NULL) {
		if (strcmp(keyboardCmd, "+") == 0) {
			nextBrightness = state->currentBrightness > 0xEFFF ?  0xFFFF : state->currentBrightness + 0x0200;
		} else if (strcmp(keyboardCmd, "-") == 0) {
			nextBrightness = state->currentBrightness < 0x1000 ?  0x0000 : state->currentBrightness - 0x0200;
		} else if (strcmp(keyboardCmd, "Z") == 0) {
			nextBrightness = 0xFFFF;
		} else if (strcmp(keyboardCmd, "X") == 0) {
			nextBrightness = 0x0000;
		} else {
			nextBrightness = state->currentBrightness;
		}
		setupLedPatternsToQueue(state, state->currentBrightness, nextBrightness,  2, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		state->lastKBInputUpdate = getCurrentUSecs();
	}

	if (webCmd != NULL) {
		printf("Web command is: %s\n", webCmd);
		if (strcmp(webCmd, "Full On") == 0) {
			setupLedPatternsToQueue(state, state->currentBrightness, 0xFFFF,  3, &ledPattern_acknowledgeCommand, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		} else if (strcmp(webCmd, "Full Off") == 0) {
			setupLedPatternsToQueue(state, state->currentBrightness, 0x0000,  3, &ledPattern_acknowledgeCommand, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		} else if (strcmp(webCmd, "Half Way") == 0) {
			setupLedPatternsToQueue(state, state->currentBrightness, 0x8000,  3, &ledPattern_acknowledgeCommand, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		} else if (strcmp(webCmd, "5 Minute Delay") == 0) {
			setupLedPatternsToQueue(state, state->currentBrightness, 0x0000,  4, &ledPattern_acknowledgeCommand, &ledPattern_fiveMinuteDelay, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		} else if (strcmp(webCmd, "Night Mode") == 0) {
			setupLedPatternsToQueue(state, state->currentBrightness, 0x0000,  4, &ledPattern_acknowledgeCommand, &ledPattern_nightMode);
		}
		state->lastWebUpdate = getCurrentUSecs();
	}

	if ((touchKey1 != NULL) && (*touchKey1 == 1)) {
		setupLedPatternsToQueue(state, state->currentBrightness, 0xFFFF,  2, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		state->lastTouchSensorUpdate = getCurrentUSecs();
	}
	if ((touchKey2 != NULL) && (*touchKey2 == 1)) {
		setupLedPatternsToQueue(state, state->currentBrightness, 0x0000,  2, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		state->lastTouchSensorUpdate = getCurrentUSecs();
	}
	if ((touchKey3 != NULL) && (*touchKey3 == 1)) {
		setupLedPatternsToQueue(state, state->currentBrightness, 0x0000,  4, &ledPattern_acknowledgeCommand, &ledPattern_fiveMinuteDelay, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		state->lastTouchSensorUpdate = getCurrentUSecs();
	}
	if ((touchKey4 != NULL) && (*touchKey4 == 1)) {
		setupLedPatternsToQueue(state, state->currentBrightness, 0x0000,  1, &ledPattern_nightMode);
		state->lastTouchSensorUpdate = getCurrentUSecs();
	}


	long long nextExpectedPatternStepTime = executePatternStep(state);
	setReturnStructureWithPatterns(&ledDriver_returnStructure, state, nextExpectedPatternStepTime);

	return &ledDriver_returnStructure;	
}

struct allSensorsDescription_t* ledDriver_actionStateAllSensors(gpointer ptr){
	return &ledDriver_allSensors;
}

const char *ledDriver_getActionName(gpointer sensorStatePtr) {
	struct ledDriver_sensorStat *sensorState = (struct ledDriver_sensorStat *) sensorStatePtr;
	return sensorState->sensorStateName;
}

void ledDriver_closeActionFunction(gpointer rawSensorState) {
	struct ledDriver_sensorStat *state = (struct ledDriver_sensorStat *) rawSensorState;
	destroyLedControll(state->ledctrl);
	spi_closeDevice(state->spiDevice);
}

struct actionDescriptorStructure_t ledDriverActionStructure = {
	.sensorType = "LedDriver",
	.sensorStatePtr = NULL,
	.initiateActionFunction = &ledDriver_initActionFunction,
	.stateWatchedInputs     = &ledDriver_actionStateWatchedInputs,
	.stateAllSensors        = &ledDriver_actionStateAllSensors,
	.actionFunction 		= &ledDriver_actionFunction,
	.getActionNameFunction  = &ledDriver_getActionName,
	.destroyActionFunction  = &ledDriver_closeActionFunction
};

#undef __APPEND_TOUCH_SENSOR_NAME
