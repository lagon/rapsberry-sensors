char *externalEventNotificationPipePath = "/tmp/ra_sen_notification_pipe";
int el_wantStop = 0;

#include "actionQueue.h"
#include "main_event_loop.h"

#define DEBUGPRINT(format, param) printf(format, param);

void registerAndInitializeSingleSensor(gpointer sensorPtr, gpointer eventLoopPtr) {
	struct actionDescriptorStructure_t *sensor = (struct actionDescriptorStructure_t *) sensorPtr;
	struct mainEventLoopControl_t *eventLoop = (struct mainEventLoopControl_t *) eventLoopPtr;
	
	const char* sensorNameOriginal = sensor->getActionNameFunction();
	char *sensorName = (char *)malloc(sizeof(char) * (strlen(sensorNameOriginal) + 10));
	sensorName = strcpy(sensorName, sensorNameOriginal);
	
	printf("Sensor %s initating.\n", sensorName);

	struct actionReturnValue_t* initReturn = sensor->initiateActionFunction();
	if (initReturn->actionErrorStatus != 0) {
		logErrorMessage("Unable to initiate sensor %s. Check previous messages for errors.", sensorName);
		free(sensorName);
		return;
	}

	g_hash_table_replace(eventLoop->allActionsRegistry, sensorName, sensor);

	struct inputNotifications_t *watchedSensors = sensor->stateWatchedInputs();
	for (int inputID = 0; inputID < watchedSensors->numInputsWatched; inputID++) {
		GList *registeredWatchers = g_hash_table_lookup(eventLoop->registeredInputWatchers, watchedSensors->watchedInputs[inputID]);
		if (registeredWatchers == NULL) {
			registeredWatchers = g_list_append(registeredWatchers, sensorName);
			g_hash_table_replace(eventLoop->registeredInputWatchers, watchedSensors->watchedInputs[inputID], registeredWatchers);
		} else {
			registeredWatchers = g_list_append(registeredWatchers, sensorName);
		}
	}

	g_hash_table_replace(eventLoop->allActionsStatuses, sensorName, initReturn->sensorState);

	aq_addAction(eventLoop->actionQueue, initReturn->usecsToNextInvocation, sensor);
	printf("Sensor %s initiation completed.\n", sensorName);
	return;
}

struct mainEventLoopControl_t* el_initializeEventLoop(GList *uninitializedSensors) {
	struct mainEventLoopControl_t* eventLoop = (struct mainEventLoopControl_t*) malloc(sizeof(struct mainEventLoopControl_t));

	eventLoop->allActionsRegistry = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->allActionsStatuses = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->actionQueue = aq_initQueue();
	eventLoop->registeredInputWatchers = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->inputValues = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->changedInputValues = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->allSensorValues = g_hash_table_new(&g_str_hash, &g_str_equal);
	printf("About to initiate actions.\n");
	//Register all sensors
	g_list_foreach(uninitializedSensors, &registerAndInitializeSingleSensor, eventLoop);

	return(eventLoop);
};

void el_readExternalInputs(struct mainEventLoopControl_t *eventLoop) {
	return;
}

void el_registerChangedInputs(GHashTable *changedInputValues, struct inputsChanged_t *changedInputs) {
	for (int inputID = 0; inputID < changedInputs->numInputsChanged; inputID++) {
//		DEBUGPRINT("##### Registering changed input - %s\n", changedInputs->newInputValues[inputID].inputName);
		g_hash_table_replace(changedInputValues, changedInputs->newInputValues[inputID].inputName, &(changedInputs->newInputValues[inputID]));
	}
}

void el_executeAction(struct actionDescriptorStructure_t *action2Execute, struct mainEventLoopControl_t *eventLoop) {
	const char *sensorName = action2Execute->getActionNameFunction();
//	printf("About to execute action %s\n", sensorName);
	gpointer *sensorState = g_hash_table_lookup(eventLoop->allActionsStatuses, sensorName);
	if (sensorState == NULL) {
		logErrorMessage("State for sensor \"%s\" was not found. Disabling sensor.\n", sensorName);
		return;
	}

	long long startTime = getCurrentUSecs();
	struct actionReturnValue_t *ret = action2Execute->actionFunction(sensorState, eventLoop->allSensorValues, eventLoop->inputValues);
	long long stopTime = getCurrentUSecs();
	g_hash_table_replace(eventLoop->allActionsStatuses, sensorName, ret->sensorState);

	el_registerChangedInputs(eventLoop->changedInputValues, ret->changedInputs);

	if ((ret->waitOnInputMode & WAIT_TIME_PERIOD)) {
//		DEBUGPRINT("\tAction %s wants to be called again in future.\n", sensorName);
		aq_addAction(eventLoop->actionQueue, ret->usecsToNextInvocation, action2Execute);
	}
	printf("It took %lld us to execute action %s\n", stopTime - startTime, sensorName);

	return;
}

int el_isAnyInputChangesWaiting(struct mainEventLoopControl_t *eventLoop) {
	return(g_hash_table_size(eventLoop->changedInputValues) != 0);
}

void el_copyInputValueToFinalDestination(gpointer inputNamePtr, gpointer newInputValuePtr, gpointer inputValuesStoragePtr) {
	g_hash_table_replace((GHashTable *) inputValuesStoragePtr, inputNamePtr, newInputValuePtr);
}

void el_updateInputValues(struct mainEventLoopControl_t * eventLoop) {
	g_hash_table_foreach(eventLoop->changedInputValues, &el_copyInputValueToFinalDestination, (gpointer) eventLoop->inputValues);
	g_hash_table_remove_all(eventLoop->changedInputValues);
}

GList *el_getActionsWaitingForChangedInputs(struct mainEventLoopControl_t *eventLoop) {
	GList *modifiedInputNames = g_hash_table_get_keys(eventLoop->changedInputValues);

	GHashTable *uniqueActionNamesToInvoke = g_hash_table_new(&g_str_hash, &g_str_equal);
	for (GList *currentInputName = modifiedInputNames; currentInputName != NULL; currentInputName = currentInputName->next) {
		GList *actionNames = g_hash_table_lookup(eventLoop->registeredInputWatchers, currentInputName->data);
//		DEBUGPRINT("\tProcessing changed input %s\n", (char *)currentInputName->data);
		for (GList *currentActionName = actionNames; currentActionName != NULL; currentActionName = currentActionName->next) {
			g_hash_table_add(uniqueActionNamesToInvoke, currentActionName->data);
//			DEBUGPRINT("\t\t+ Adding action %s\n", (char *)currentActionName->data);
		}
	}

	GList *actionNames2Notify = g_hash_table_get_keys(uniqueActionNamesToInvoke);
	GList *actions2Notify = NULL;
	for (GList *actionName = actionNames2Notify; actionName != NULL; actionName = actionName->next) {
		actions2Notify = g_list_append(actions2Notify, g_hash_table_lookup(eventLoop->allActionsRegistry, actionName->data));
	}
	
	g_hash_table_destroy(uniqueActionNamesToInvoke);
	g_list_free(actionNames2Notify);
	g_list_free(modifiedInputNames);
	return actions2Notify;
}


void el_runEventLoop(struct mainEventLoopControl_t *eventLoop) {
	char ch;
	if (mkfifo(externalEventNotificationPipePath, 0666) < 0) {
		if (errno != EEXIST) {
			logErrorMessage("Unable to create external notification pipe. Reason: %s", strerror(errno));
			return;
		};
	};
	int externalEventNotifierPipe = open(externalEventNotificationPipePath, O_NONBLOCK);
	if (externalEventNotifierPipe < 0) {
		logErrorMessage("Unable to open external notification pipe. Reason: %s", strerror(errno));
		return;
	}

	fd_set externalEventNotifiersToWatch;
	FD_ZERO(&externalEventNotifiersToWatch);
	FD_SET(externalEventNotifierPipe, &externalEventNotifiersToWatch);

	struct timeval waitingTime;

	while (el_wantStop == 0) {
		int bytesRead = read(externalEventNotifierPipe, &ch, 1);
		if (bytesRead > 0) {
			el_readExternalInputs(eventLoop);
		}

		int isInputChanged = el_isAnyInputChangesWaiting(eventLoop);
		GList *actions2Notify = el_getActionsWaitingForChangedInputs(eventLoop);
		el_updateInputValues(eventLoop);

		if (isInputChanged) {
			for (GList *action = actions2Notify; action != NULL; action = action->next) {
				el_executeAction(action->data, eventLoop);
			}
		} else if (aq_usecsToNextAction(eventLoop->actionQueue) <= 100) {
			struct actionDescriptorStructure_t *action = aq_getAction(eventLoop->actionQueue);
			el_executeAction(action, eventLoop);
		};

		g_list_free(actions2Notify);
		

		if (!el_isAnyInputChangesWaiting(eventLoop)) {
//			DEBUGPRINT("There is no new input waiting to be processed...\n", "");
			long long usecsToNextAction = aq_usecsToNextAction(eventLoop->actionQueue);
			waitingTime.tv_usec = usecsToNextAction;
			waitingTime.tv_sec = 0;
			select(externalEventNotifierPipe + 1, &externalEventNotifiersToWatch, NULL, NULL, &waitingTime);
		}
	}
}