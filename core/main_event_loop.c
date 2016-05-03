
#include "sensorConfigParser.h"
#include "actionQueue.h"
#include "main_event_loop.h"

#define DEBUGPRINT(format, param) printf(format, param);

char *externalEventNotificationPipePath = "/tmp/ra_sen_notification_pipe";
int el_wantStop = 0;

void freeChangedInputsStructure(struct inputsChanged_t *changedInputs);

struct actionDescriptorStructure_t *allocateAndCopyActionDescriptionStructure(struct actionDescriptorStructure_t *sensor) {
	struct actionDescriptorStructure_t *cpy = (struct actionDescriptorStructure_t *) malloc(sizeof(struct actionDescriptorStructure_t));
	memcpy(cpy, sensor, sizeof(struct actionDescriptorStructure_t));
	return cpy;
}

void registerAndInitializeSingleSensor(struct mainEventLoopControl_t *eventLoop, struct actionDescriptorStructure_t *sensor, struct sensorConfig_t *cfg) {
	printf("Sensor of type %s is initating.\n", sensor->sensorType);
	struct actionReturnValue_t* initReturn = sensor->initiateActionFunction(cfg->sensorNameAppendix, cfg->sensorAddress, cfg->sensorOptions);
	if (initReturn->actionErrorStatus != 0) {
		char *str = allocateAndConcatStrings(sensor->sensorType, cfg->sensorNameAppendix);
		logErrorMessage("Unable to initiate sensor %s. Check previous messages for errors.", str);
		free(str);
		return;
	}
	sensor->sensorStatePtr = initReturn->sensorState;
	const char* sensorNameOriginal = sensor->getActionNameFunction(sensor->sensorStatePtr);
	char *sensorName = (char *)malloc(sizeof(char) * (strlen(sensorNameOriginal) + 10));
	sensorName = strcpy(sensorName, sensorNameOriginal);
	
	printf("Sensor is registered as %s.\n", sensorName);

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
	freeChangedInputsStructure(initReturn->changedInputs);

	struct actionDescriptorStructure_t *sensorCpy = allocateAndCopyActionDescriptionStructure(sensor);

	aq_addAction(eventLoop->actionQueue, initReturn->usecsToNextInvocation, sensorCpy);
	printf("Sensor %s initiation completed.\n", sensorName);
	return;
}

void freeOutdatedSensorValue(gpointer ptr) {
	free(ptr);
}

void freeOutdatedInputValue(gpointer ptr) {
	struct inputValue_t * input = (struct inputValue_t *)ptr;
	if (input->type == InputTypeString) {
		free(input->stringValue);
	}
	free(input->inputName);
	free(ptr);
}

struct mainEventLoopControl_t* el_initializeEventLoop(GHashTable *allActions, GList *configuredActions) {
	struct mainEventLoopControl_t *eventLoop = (struct mainEventLoopControl_t*) malloc(sizeof(struct mainEventLoopControl_t));

	eventLoop->allActionsRegistry = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->allActionsStatuses = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->actionQueue = aq_initQueue();
	eventLoop->registeredInputWatchers = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->inputValues = g_hash_table_new_full(&g_str_hash, &g_str_equal, &freeOutdatedSensorValue, &freeOutdatedInputValue);
	eventLoop->changedInputValues = g_hash_table_new(&g_str_hash, &g_str_equal);
	eventLoop->allSensorValues = g_hash_table_new_full(&g_str_hash, &g_str_equal, NULL, &freeOutdatedSensorValue);
	
	printf("About to initiate actions.\n");
	//Register all sensors

	GList *configuredItem;
	for (configuredItem = configuredActions; configuredItem != NULL; configuredItem = configuredItem->next) {
		struct sensorConfig_t *cfg = (struct sensorConfig_t *) configuredItem->data;
		struct actionDescriptorStructure_t *sensor = (struct actionDescriptorStructure_t *) g_hash_table_lookup(allActions, cfg->sensorType);
		if (sensor == NULL) {
			printf("There is no sensor of type %s\n", cfg->sensorType);
			continue;
		}
		registerAndInitializeSingleSensor(eventLoop, sensor, cfg);
	}

	return(eventLoop);

};

struct inputsChanged_t *transformExternalInputs(GList *externalInputList) {
	struct inputsChanged_t *externalInputs = (struct inputsChanged_t *) malloc(sizeof(struct inputsChanged_t) + sizeof(struct inputValue_t) * g_list_length(externalInputList));
	externalInputs->numInputsChanged = g_list_length(externalInputList);

	int inputID = 0;
	for (GList *item = externalInputList; item != NULL; item = item->next) {
		struct inputValue_t *inp = (struct inputValue_t *)item->data;
		externalInputs->newInputValues[inputID].inputName = (char *)malloc((strlen(inp->inputName) + 1) * sizeof(char));
		strcpy(externalInputs->newInputValues[inputID].inputName, inp->inputName);
		externalInputs->newInputValues[inputID].valueMeasuredTimestamp = inp->valueMeasuredTimestamp;
		externalInputs->newInputValues[inputID].type = inp->type;
		if (externalInputs->newInputValues[inputID].type == InputTypeString) {
			externalInputs->newInputValues[inputID].stringValue = (char *)malloc((strlen(inp->stringValue) + 1) * sizeof(char));
			strcpy(externalInputs->newInputValues[inputID].stringValue, inp->stringValue);
		} else if (externalInputs->newInputValues[inputID].type == InputTypeInteger) {
			externalInputs->newInputValues[inputID].integerValue = inp->integerValue;
		} else if (externalInputs->newInputValues[inputID].type == InputTypeDouble) {
			externalInputs->newInputValues[inputID].doubleValue = inp->doubleValue;
		}
		printf("Transformed %s \n", externalInputs->newInputValues[inputID].inputName);
		inputID++;
	}
	
	printf("Transformed %d inputs from external.\n", externalInputs->numInputsChanged);	
	return externalInputs;
}

struct inputValue_t *copyInputValue(struct inputValue_t *original) {
	struct inputValue_t *copy = (struct inputValue_t *)malloc(sizeof(struct inputValue_t));
	copy->inputName = (char *) malloc(sizeof(char) * (strlen(original->inputName) + 1));
	strcpy(copy->inputName, original->inputName);
	copy->valueMeasuredTimestamp = original->valueMeasuredTimestamp;
	copy->type = original->type;
	if (copy->type == InputTypeString) {
		copy->stringValue = (char *)malloc(sizeof(char) * (strlen(original->stringValue) + 1));
		strcpy(copy->stringValue, original->stringValue);
	} else if (copy->type == InputTypeDouble) {
		copy->doubleValue = copy->doubleValue;
	} else if (copy->type == InputTypeInteger) {
		copy->integerValue = original->integerValue;
	}
	return copy;
}

void freeChangedInputsStructure(struct inputsChanged_t *changedInputs) {
	for  (int inputID = 0; inputID < changedInputs->numInputsChanged; inputID++) {
		if(changedInputs->newInputValues[inputID].type == InputTypeString) {
			free(changedInputs->newInputValues[inputID].stringValue);
		}
		free(changedInputs->newInputValues[inputID].inputName);
	}
	free(changedInputs);
}


void el_registerChangedInputs(GHashTable *changedInputValues, struct inputsChanged_t *changedInputs) {
	for (int inputID = 0; inputID < changedInputs->numInputsChanged; inputID++) {
		char *inputNameKey = (char *)malloc((strlen(changedInputs->newInputValues[inputID].inputName) + 1) * sizeof(char));
		strcpy(inputNameKey, changedInputs->newInputValues[inputID].inputName);
		gpointer value = copyInputValue(&(changedInputs->newInputValues[inputID]));
		DEBUGPRINT("##### Registering changed input - %s\n", inputNameKey);
		g_hash_table_replace(changedInputValues, inputNameKey, value);
	}
	freeChangedInputsStructure(changedInputs);
}

void freeExternalInputs(gpointer data, gpointer userData) {
	free(data);
}

void el_readExternalInputs(struct mainEventLoopControl_t *eventLoop) {
	GList *externalInputsList = readExternalInputsFromDb();

	struct inputsChanged_t *externalInputs = transformExternalInputs(externalInputsList);

	el_registerChangedInputs(eventLoop->changedInputValues, externalInputs);
	
	g_list_foreach(externalInputsList, &freeExternalInputs, NULL);
	g_list_free(externalInputsList);

	for (int inputID = 0; inputID < externalInputs->numInputsChanged; inputID++) {
		free(externalInputs->newInputValues[inputID].inputName);
		if (externalInputs->newInputValues[inputID].type == InputTypeString) {
			free(externalInputs->newInputValues[inputID].stringValue);
		}
	}
}

void el_executeAction(struct actionDescriptorStructure_t *action2Execute, struct mainEventLoopControl_t *eventLoop) {
	const char *sensorName = action2Execute->getActionNameFunction(action2Execute->sensorStatePtr);
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
	//printf("It took %lld us to execute action %s... \n", stopTime - startTime, sensorName);

	return;
}

int el_isAnyInputChangesWaiting(struct mainEventLoopControl_t *eventLoop) {
	if(g_hash_table_size(eventLoop->changedInputValues) > 0) {
		printf("There is %d inputs waiting\n", g_hash_table_size(eventLoop->changedInputValues));
	}
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
		printf("\tProcessing changed input %s, (key is %s)\n", (char *)currentInputName->data, "X");
		for (GList *currentActionName = actionNames; currentActionName != NULL; currentActionName = currentActionName->next) {
			g_hash_table_add(uniqueActionNamesToInvoke, currentActionName->data);
			DEBUGPRINT("\t\t+ Adding action %s\n", (char *)currentActionName->data);
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
			logErrorMessage("Unable to create external notification pipe. Reason: %s \n", strerror(errno));
			_exit(-1);
		} else {
			printf("Notification pipe already exists. \n");
		}
	};
	if (chmod(externalEventNotificationPipePath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) < 0) {
		logErrorMessage("Unable to allow access for notification pipe: %s \n", strerror(errno));
		_exit(-1);
	}

	int externalEventNotifierPipe = open(externalEventNotificationPipePath, O_RDONLY | O_NONBLOCK);
	if (externalEventNotifierPipe < 0) {
		logErrorMessage("Unable to open external notification pipe. Reason: %s", strerror(errno));
		return;
	}

	int externalEventNotifierPipeWriterEnd = open(externalEventNotificationPipePath, O_WRONLY);
	if (externalEventNotifierPipeWriterEnd < 0) {
		logErrorMessage("Unable to open external notification pipe. Reason: %s", strerror(errno));
		return;
	}


	fd_set externalEventNotifiersToWatch;
	FD_ZERO(&externalEventNotifiersToWatch);
	FD_SET(externalEventNotifierPipe, &externalEventNotifiersToWatch);

	struct timeval waitingTime;
//	int loopCnt = 100;

	while (el_wantStop == 0) {
		int bytesRead = read(externalEventNotifierPipe, &ch, 1);
		//printf("Bytes from notifier pipe: %d\n", bytesRead);
		
	 	if (bytesRead > 0) {
			printf("*********************************************\nThere is an input waiting in BD\n*********************************************\n");
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
			long long usecsToNextAction = aq_usecsToNextAction(eventLoop->actionQueue);
//			printf(" no inputs -> sleeping for %'lld usec...\n", usecsToNextAction);

			waitingTime.tv_usec = usecsToNextAction;
			waitingTime.tv_sec = 0;
			FD_ZERO(&externalEventNotifiersToWatch);
			FD_SET(externalEventNotifierPipe, &externalEventNotifiersToWatch);

			int selectRet = select(externalEventNotifierPipe + 100	, &externalEventNotifiersToWatch, NULL, NULL, &waitingTime);
			// perror("Troubles in select() ");
			// printf("Select returned %d\n", selectRet);
			// printf("%d (notification file is set?) %d \n", externalEventNotifierPipe, FD_ISSET(externalEventNotifierPipe, &externalEventNotifiersToWatch));
			if (selectRet < 0) {return;}

		} else {
			printf("There is some input to be processed...\n");
		}
		// if (loopCnt < 1) {
		// 	break;
		// }
		// loopCnt--;
	}
}