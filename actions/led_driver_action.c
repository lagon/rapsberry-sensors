#include "led_driver_action.h"
#include "led_driver_library.h"
#include "kbInput_action.h"
#include "utilityFunctions.h"
#include <glib.h>
#include "pattern_queue.h"
#include "all_led_patterns.h"
#include <stdarg.h>

const char *ledDriverActionName = "LedDriveAction";
const int numLeds = 12;

struct allSensorsDescription_t ledDriver_allSensors = {
	.numSensors = 0,
	.sensorDescriptions = {}
};

struct actionReturnValue_t ledDriver_returnStructure;

struct inputNotifications_t ledDrive_watchedInputs = {
	.numInputsWatched = 2,
	.watchedInputs = {"chodba_webInput", __keyboardInputName}
};

struct ledDriver_sensorStat {
	int spiDevice;
	struct allLedControlStruct *ledctrl;
	long long lastKBInputUpdate;
	long long lastWebUpdate;
	uint16_t currentBrightness;
	struct pa_Queue *ledPatternsQueue;
	int ledPatternTimeStep;
} ledDriver_sensorStat;

void setReturnStructure(struct actionReturnValue_t *returnStructure, struct ledDriver_sensorStat *sensorState);

void setLedsAccordingToPattern(struct allLedControlStruct *ledctrl, struct pa_LedStatesResults *pattern) {
	for (int ledID = 0; ledID < ledctrl->numLeds; ledID++) {
		setOneGrayscaleLed(ledctrl, ledID, pattern->ledIntensities[ledID]);
	}
}

struct actionReturnValue_t* ledDriver_initActionFunction() {
	int spidev = spi_initDevice(0, 0);
	if (spidev < 0) {
		perror("");
		exit(-1);
	}

	struct allLedControlStruct *ledctrl = initiateLEDControls(1);
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

char *checkNewInput(char *inputName, long long lastSeenUpdate, GHashTable* allInputs) {
	struct inputValue_t *input = (struct inputValue_t *)g_hash_table_lookup(allInputs, inputName);
	if (input == NULL) {
		return NULL;
	}
	if (input->valueMeasuredTimestamp <= lastSeenUpdate) {
		return NULL;
	}
	return input->stringValue;
}

void setupLedPatternsToQueue(struct ledDriver_sensorStat *state, uint16_t initialBrightness, uint16_t targetBrightness, int numPatternsToAdd, ...) {
	va_list valist;
	pa_resetQueue(state->ledPatternsQueue);
	va_start(valist, numPatternsToAdd);
	for (int patternID = 0; patternID < numPatternsToAdd; patternID++) {
		pattern_action pattern = va_arg(valist, pattern_action);
		pa_addNextLedAction(state->ledPatternsQueue, pattern, initialBrightness, targetBrightness);
	}
	va_end(valist);
	state->ledPatternTimeStep = 0;
}

long long executePatternStep(struct ledDriver_sensorStat *state) {
	if (pa_isQueueEmpty(state->ledPatternsQueue)) {
		return pa_neverCallAgain;
	}

	struct pa_LedStatesResults *patternStep = pa_executeCurrentLedAction(state->ledPatternsQueue, state->ledPatternTimeStep);

	long long timeToNextInvodation = patternStep->nextInvocation;
	if (timeToNextInvodation == pa_wasLastStep) {
		pa_getToNextLedAction(state->ledPatternsQueue);
		state->ledPatternTimeStep = 0;
		timeToNextInvodation = 500 * 1000;
	} else if (timeToNextInvodation == pa_neverCallAgain) {
		pa_resetQueue(state->ledPatternsQueue);
	} else if (timeToNextInvodation == pa_emptyActionQueu) {
		pa_destroyLedStateResults(patternStep);
		return -1;
	} else {
		state->ledPatternTimeStep = state->ledPatternTimeStep + 1;
	}
	setLedsAccordingToPattern(state->ledctrl, patternStep);
	sendOutLedDataDefaults(state->ledctrl, state->spiDevice);

	state->currentBrightness = patternStep->ledIntensities[0];
	pa_destroyLedStateResults(patternStep);
	return timeToNextInvodation;
}

struct actionReturnValue_t* ledDriver_actionFunction(gpointer rawSensorState, GHashTable* measurementOutput, GHashTable* allInputs) {
	struct ledDriver_sensorStat *state = (struct ledDriver_sensorStat *) rawSensorState;

	char *keyboardCmd = checkNewInput(__keyboardInputName, state->lastKBInputUpdate, allInputs);
	char *webCmd      = checkNewInput("chodba_webInput",   state->lastWebUpdate, allInputs);

	if ((keyboardCmd == NULL) && (webCmd == NULL)) {
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
			uint16_t nextBrightness = state->currentBrightness > 0xEFFF ?  0xFFFF : state->currentBrightness + 0x0200;
		else if (strcmp(keyboardCmd, "-") == 0) {
			uint16_t nextBrightness = state->currentBrightness < 0x1000 ?  0x0000 : state->currentBrightness - 0x0200;
		} else {
			uint16_t nextBrightness = state->currentBrightness;
		}
		setupLedPatternsToQueue(state->ledPatternsQueue, state->currentBrightness, nextBrightness,  2, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		state->lastKBInputUpdate = getCurrentUSecs();
	}

	if (webCmd != NULL) {
		printf("Web command is: %s\n", webCmd);
		if (strcmp(webCmd, "full on") == 0) {
			setupLedPatternsToQueue(state->ledPatternsQueue, state->currentBrightness, 0xFFFF,  3, &ledPattern_acknowledgeCommand, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		} else if (strcmp(webCmd, "full off") == 0) {
			setupLedPatternsToQueue(state->ledPatternsQueue, state->currentBrightness, 0x0000,  3, &ledPattern_acknowledgeCommand, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		} else if (strcmp(webCmd, "half way") == 0) {
			setupLedPatternsToQueue(state->ledPatternsQueue, state->currentBrightness, 0x8000,  3, &ledPattern_acknowledgeCommand, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		} else if (strcmp(webCmd, "5 minute delay") == 0) {
			setupLedPatternsToQueue(state->ledPatternsQueue, state->currentBrightness, 0x0000,  3, &ledPattern_acknowledgeCommand, &ledPattern_setIntensityMediumFade, &ledPattern_setIntensityInOneStep);
		}
		state->lastWebUpdate = getCurrentUSecs();
	}


	long long nextExpectedPatternStepTime = executePatternStep(state);
	setReturnStructureWithPatterns(&ledDriver_returnStructure, state, nextExpectedPatternStepTime);

	return &ledDriver_returnStructure;	
}

struct allSensorsDescription_t* ledDriver_actionStateAllSensors(){
	return &ledDriver_allSensors;
}

const char *ledDriver_getActionName() {
	return ledDriverActionName;
}

void ledDriver_closeActionFunction(gpointer rawSensorState) {
	struct ledDriver_sensorStat *state = (struct ledDriver_sensorStat *) rawSensorState;
	destroyLedControll(state->ledctrl);
	spi_closeDevice(state->spiDevice);
}

struct actionDescriptorStructure_t ledDriverActionStructure = {
	.initiateActionFunction = &ledDriver_initActionFunction,
	.stateWatchedInputs     = &ledDriver_actionStateWatchedInputs,
	.stateAllSensors        = &ledDriver_actionStateAllSensors,
	.actionFunction 		= &ledDriver_actionFunction,
	.getActionNameFunction  = &ledDriver_getActionName,
	.destroyActionFunction  = &ledDriver_closeActionFunction
};

