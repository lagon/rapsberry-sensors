#ifndef __lagon_main_event_loop_h__
#define __lagon_main_event_loop_h__

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <sqlite3.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>

#include "utilityFunctions.h"
#include "actionDescriptorStructure.h"
#include "sensorDescriptionStructure.h"
#include "sqliteDbUtilityFunctions.h"

struct mainEventLoopControl_t {
	GHashTable *allActionsRegistry; // Hastable with action name string as a key and pointer to struct actionDescriptorStructure_t. Contains all the actions registered even those no longer scheduled.
	GHashTable *allActionsStatuses; // Hashtable with action name string as key and pointer to action status as value. Used to hold current statuses of individual actions.
	struct actionQueue *actionQueue; // Queue of scheduled actions in time.

	GHashTable *registeredInputWatchers; // Input name as key and pointer array (see glib) containing action names (as registered in allActionsRegistry).
	GHashTable *allSensorValues; //Current values of all sensor. Key is string name and value is double.

	GHashTable *inputValues; //Latest values on individual inputs. Key is name of the input and value is inputValue_t.
	GHashTable *changedInputValues; //Updated values of individual inputs. Key is name of the input and value is inputValue_t. At the end of action notification cycle, values will be moved from this hash table to "main" inputValues table.

	sqlite3 *externalInputs;
};

struct mainEventLoopControl_t* el_initializeEventLoop(GList *initializedSensors);
void el_runEventLoop(struct mainEventLoopControl_t *eventLoop);

#endif