#include <sys/select.h>
#include <malloc.h>
#include <glib.h>
#include <termios.h>
#include <unistd.h>

#include "kbInput_action.h"
#include "actionDescriptorStructure.h"
#include "utilityFunctions.h"


enum kbInput_blockingState_t {kbInput_BlockingTerminal, kbInput_NonBlockingTerminal} bmp183_measurementState;
const long long kbInput_KeyboardCheckingTime = 10 * 1000 * 1000; //500 ms
const char *kbInput_actionName = "kbInputAction";
const char *keyboardInputName = __keyboardInputName;
const int stdinFileID = 0;

struct actionReturnValue_t kbInput_returnStructure;

struct allSensorsDescription_t kbInput_noSensors = {
	.numSensors = 0,
	.sensorDescriptions = {}
};

int isKbHit() {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(stdinFileID, &fds); //stdinFileID is 0
    select(stdinFileID+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(stdinFileID, &fds);
}

void setTerminalNonBlockMode(int state) {
    struct termios ttystate;
    //get the terminal state
    tcgetattr(stdinFileID, &ttystate);
    if (state==kbInput_NonBlockingTerminal) {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    } else if (state==kbInput_BlockingTerminal) {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(stdinFileID, TCSANOW, &ttystate);
}

struct actionReturnValue_t* kbInput_initActionFunction() {
	char *lastChar = (char *)malloc(sizeof(char));

	setTerminalNonBlockMode(kbInput_NonBlockingTerminal);
	kbInput_returnStructure.sensorState = lastChar;
	kbInput_returnStructure.actionErrorStatus = 0;
	kbInput_returnStructure.usecsToNextInvocation = kbInput_KeyboardCheckingTime;
	kbInput_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	kbInput_returnStructure.changedInputs = &noInputsChanged;

	return &kbInput_returnStructure;
}

struct inputNotifications_t *kbInput_actionStateWatchedInputs() {
	return &noInputsToWatch;
}

struct allSensorsDescription_t* kbInput_actionStateAllSensors() {
	return &kbInput_noSensors;
}

struct inputsChanged_t* createInputStructure(char ch) {
	struct inputsChanged_t* inpCh = (struct inputsChanged_t*) malloc(sizeof(struct inputsChanged_t) + sizeof(struct inputValue_t));
	inpCh->numInputsChanged = 1;
	inpCh->newInputValues[0].inputName = keyboardInputName;
	inpCh->newInputValues[0].valueMeasuredTimestamp = getCurrentUSecs();
	inpCh->newInputValues[0].type = InputTypeInteger;
	inpCh->newInputValues[0].integerValue = ch;

	return(inpCh);
}

struct actionReturnValue_t* kbInput_actionFunction(gpointer rawSensorStatus, GHashTable* measurementOutput, GHashTable *allInputs) {
	kbInput_returnStructure.sensorState = rawSensorStatus;
	kbInput_returnStructure.actionErrorStatus = 0;
	kbInput_returnStructure.usecsToNextInvocation = kbInput_KeyboardCheckingTime;	
	kbInput_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	

	if (isKbHit()) {
		char ch = ' ';
		read(stdinFileID, &ch, 1);
//		printf("Char read from keyboard %c\n", ch);
		kbInput_returnStructure.changedInputs = createInputStructure(ch);
		char *lastChar = (char *)rawSensorStatus;
		*lastChar = ch;
		g_hash_table_replace(allInputs, keyboardInputName, lastChar);
	} else {
		kbInput_returnStructure.changedInputs = &noInputsChanged;
//		printf("No key was pressed\n");
	}

	return &kbInput_returnStructure;
}

const char *kbInput_getActionName() {
	return kbInput_actionName;
}

void kbInput_closeActionFunction(gpointer rawSensorStatus) {
	setTerminalNonBlockMode(kbInput_BlockingTerminal);
	free(rawSensorStatus);
}

struct actionDescriptorStructure_t kbInputActionStructure = {
	.initiateActionFunction = &kbInput_initActionFunction,
	.stateWatchedInputs     = &kbInput_actionStateWatchedInputs,
	.stateAllSensors        = &kbInput_actionStateAllSensors,
	.actionFunction 		= &kbInput_actionFunction,
	.getActionNameFunction  = &kbInput_getActionName,
	.destroyActionFunction  = &kbInput_closeActionFunction
};
