#ifndef __action_descriptor_structure__
#define __action_descriptor_structure__

#include <glib.h>

#include "sensorDescriptionStructure.h"

struct inputNotifications_t {
	int numInputsWatched;
	char *watchedInputs[];
} inputNotifications_t;

enum inputValueType_t {InputTypeString, InputTypeDouble, InputTypeInteger} inputValueType;
struct inputValue_t {
	char *inputName;
	long long valueMeasuredTimestamp;
	enum inputValueType_t type;
	union {
		char *stringValue;
		int integerValue;
		double doubleValue;
 	};
} inputValue_t;


struct inputsChanged_t {
	int numInputsChanged;
	struct inputValue_t newInputValues[];
} inputsChanged_t;



#define WAIT_ON_INPUT 1
#define WAIT_TIME_PERIOD 2
#define WAIT_TIME_PERIOD_OR_INPUT 3

struct actionReturnValue_t {
	gpointer sensorState;
	int actionErrorStatus;
	long usecsToNextInvocation;
	int waitOnInputMode;
	struct inputsChanged_t *changedInputs;
} actionReturnValue_t;

 //Initiates the action. Returns action return type. On success the actionReturnValue_t::actionErrorStatus == 0 and sensorState set to approariate state - it will be stored and passed to action function, otherwise != 0.
typedef struct actionReturnValue_t* (*initiateActionFunction_t)(char *nameAppendix, char *address);

//Returns an array of watched inputs
typedef struct inputNotifications_t* (*stateWatchedInputs_t)();

//Actually runs the action (sensor reading, achiving the current state). It takes three parameters:
// 1) sensor state as recieved from sensor state hash map
// 2) hash table where to store actual values of all sensors
// 3) hash table with available inputs. There are all inputs and their states.
// Returns action return type. On success the actionReturnValue_t::actionErrorStatus == 0, otherwise != 0
typedef struct actionReturnValue_t* (*actionFunction_t)(gpointer, GHashTable*, GHashTable*);

//Returns all sensors modified by this action. Useful to fill in the long storage database.
typedef struct allSensorsDescription_t* (*stateAllSensors_t)();

//Returns display name of the sensor. This name will also be used to store the sensor state, so it must be unique.
typedef const char *(*getActionNameFunction_t)();

//Caled when event loop is done. Should free all the resources allocated by the action.
typedef void (*destroyActionFunction_t)(gpointer);


struct actionDescriptorStructure_t {
	char *sensorType;
	gpointer sensorStatePtr;
	initiateActionFunction_t initiateActionFunction;
	stateWatchedInputs_t stateWatchedInputs;
	stateAllSensors_t stateAllSensors;
	actionFunction_t actionFunction;
	getActionNameFunction_t getActionNameFunction;
	destroyActionFunction_t destroyActionFunction;
} actionDescriptorStructure_t;


//Convenience values for empty inputs/changed inputs...
extern struct inputNotifications_t noInputsToWatch;

struct inputsChanged_t *generateNoInputsChanged();

#endif
