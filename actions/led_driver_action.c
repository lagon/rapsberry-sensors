#include "led_driver_action.h"
#include "led_driver_library.h"
#include "kbInput_action.h"
#include "utilityFunctions.h"
#include <glib.h>


const char *ledDriverActionName = "LedDriveAction";
const int numLeds = 12;

struct allSensorsDescription_t ledDriver_allSensors = {
	.numSensors = 0,
	.sensorDescriptions = {}
};

struct actionReturnValue_t ledDriver_returnStructure;

struct inputNotifications_t ledDrive_watchedInputs = {
	.numInputsWatched = 1,
	.watchedInputs = {__keyboardInputName}
};

struct ledDriver_sensorStat {
	int spiDevice;
	struct allLedControlStruct *ledctrl;
	long long lastKBInputUpdate;
	uint16_t overallBrightness;
} ledDriver_sensorStat;

void setReturnStructure(struct actionReturnValue_t *returnStructure, struct ledDriver_sensorStat *sensorState);


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
	led->overallBrightness = 0;

	setReturnStructure(&ledDriver_returnStructure, led);
	return &ledDriver_returnStructure;
}

void setReturnStructure(struct actionReturnValue_t *returnStructure, struct ledDriver_sensorStat *sensorState) {
	returnStructure->sensorState = (gpointer) sensorState;
	returnStructure->actionErrorStatus = 0;
	returnStructure->usecsToNextInvocation = 500 * 1000;
	returnStructure->waitOnInputMode = WAIT_ON_INPUT;
	returnStructure->changedInputs = &noInputsChanged;
	return;
}


struct inputNotifications_t* ledDriver_actionStateWatchedInputs() {
	return &ledDrive_watchedInputs;
}

struct actionReturnValue_t* ledDriver_actionFunction(gpointer rawSensorState, GHashTable* measurementOutput, GHashTable* allInputs) {
	struct inputValue_t *keyboardInput = (struct inputValue_t *)g_hash_table_lookup(allInputs, __keyboardInputName);
	struct ledDriver_sensorStat *state = (struct ledDriver_sensorStat *) rawSensorState;

	if (keyboardInput == NULL) {
//		logErrorMessage("No keyboard input was defined...", "");
		setReturnStructure(&ledDriver_returnStructure, state);
		return &ledDriver_returnStructure;
	}

	if (keyboardInput->valueMeasuredTimestamp <= state->lastKBInputUpdate) {
		setReturnStructure(&ledDriver_returnStructure, state);
		return &ledDriver_returnStructure;
	}

	char key = keyboardInput->integerValue;
	printf("LED DRIVER - key read from input was - %c\n", key);
	if (key == '+') {
		state->overallBrightness = 0xFFFF;
		printf("UP\n");
		// state->overallBrightness += 1;
		// state->overallBrightness = state->overallBrightness > 10 ? 10 : state->overallBrightness;
	} else if (key == '-') {
		state->overallBrightness = 0x0000;
		printf("DOWN\n");
		// state->overallBrightness -= 1;
		// state->overallBrightness = state->overallBrightness < 0 ? 0 : state->overallBrightness;
	}

	for (int ledID = 0; ledID < numLeds; ledID++) {
		setOneGrayscaleLed(state->ledctrl, ledID, state->overallBrightness);
	}
	sendOutLedDataDefaults(state->ledctrl, state->spiDevice);


	state->lastKBInputUpdate = getCurrentUSecs();
	setReturnStructure(&ledDriver_returnStructure, state);
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

