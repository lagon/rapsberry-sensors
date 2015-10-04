#include "actionQueue.h"
#include "mcp9808_action.h"
#include "actionDescriptorStructure.h"

//const char* mcp9808TemperatureSensorName = "MCP9808 High Accuracy Temperature Sensor";
const char* mcp9808TemperatureSensorStateName = "MCP9808";

#define __mcp9808TemperatureName "MCP9808 Temperature"
const char *mcp9808TemperatureName = __mcp9808TemperatureName;

const long long mcp9808_temperatureMeasurementTime = 300 * 1000; //300ms
const long long mcp9808_idleTime = 60 * 1000 * 1000; //60 sec
const uint8_t measurementsInBurst = 5;

enum mcp9808_measurementState {MCP9808_IDLE, MCP9808_MEASURING} mcp9808_measurementState;

struct mcp9808_sensorStat {
	struct mcp9808State *device;
	int remainingMeasurementsInBurst;
	double totalTemperature;
	double temperature;
	enum mcp9808_measurementState state;

	struct allSensorsDescription_t *allSensors;
	char *sensorStateName;
	char *sensorName;
} ina219_sensorStat;

struct allSensorsDescription_t mcp9808_allSensors = {
	.numSensors = 1,
	.sensorDescriptions = {{
			.sensorID = __mcp9808TemperatureName, 
			.sensorDisplayName = "High Precision Temperature (MCP9808)", 
			.sensorUnits = "C", 
			.sensorValueName = "Temperature"
		}}
};

struct actionReturnValue_t mcp9808_returnStructure;

struct actionReturnValue_t *mcp9808_initActionFunction(char *nameAppendix, char *address) {
	struct mcp9808_sensorStat *sensor = (struct mcp9808_sensorStat *) malloc(sizeof(struct mcp9808_sensorStat));
	sensor->device = mcp9808_initTemperatureSensor(1, 0x18);
	mcp9808_stopMeasuring(sensor->device);
	sensor->state = MCP9808_IDLE;
	sensor->allSensors = constructAllSensorDescription(&mcp9808_allSensors, nameAppendix);
	sensor->sensorStateName = allocateAndConcatStrings(mcp9808TemperatureSensorStateName, nameAppendix);
	sensor->sensorName = allocateAndConcatStrings(mcp9808TemperatureName, nameAppendix);

	mcp9808_returnStructure.sensorState = sensor;
	mcp9808_returnStructure.actionErrorStatus = 0;
	mcp9808_returnStructure.usecsToNextInvocation = mcp9808_temperatureMeasurementTime;
	mcp9808_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	mcp9808_returnStructure.changedInputs = generateNoInputsChanged();

	return &mcp9808_returnStructure;
}

struct actionReturnValue_t* mcp9808_actionFunction(gpointer mcp9808SensorStat, GHashTable* measurementOutput, GHashTable *sensorStatus) {
	struct mcp9808_sensorStat *mcp9808 = (struct mcp9808_sensorStat *)mcp9808SensorStat;

	mcp9808_returnStructure.actionErrorStatus = 0;
	mcp9808_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
	mcp9808_returnStructure.changedInputs = generateNoInputsChanged();

	if (mcp9808->state == MCP9808_IDLE) {
		mcp9808_startMeasuring(mcp9808->device);
		mcp9808->totalTemperature = 0;
		mcp9808->remainingMeasurementsInBurst = measurementsInBurst;
		mcp9808->state = MCP9808_MEASURING;

		mcp9808_returnStructure.sensorState = mcp9808;
		mcp9808_returnStructure.usecsToNextInvocation = mcp9808_temperatureMeasurementTime;
	} else {
		double t = mcp9880_readTemperature(mcp9808->device);
		mcp9808->totalTemperature += t;
		mcp9808->remainingMeasurementsInBurst--;
		printf("Actual temperature is %f. %d measuements of %d to go in this burst. \n", t, mcp9808->remainingMeasurementsInBurst, measurementsInBurst);
		if (mcp9808->remainingMeasurementsInBurst <= 0) {
			mcp9808->state = MCP9808_IDLE;

			mcp9808->temperature = mcp9808->totalTemperature / (double)measurementsInBurst;
			mcp9808_stopMeasuring(mcp9808->device);
			printf("Reported temperature %f\n", mcp9808->temperature);

			struct actionOutputItem *ao_temp = generateOutputItem(mcp9808->sensorName, mcp9808->temperature);
			g_hash_table_replace(measurementOutput, (gpointer) mcp9808->sensorName, ao_temp);

			mcp9808_returnStructure.sensorState = mcp9808;
			mcp9808_returnStructure.usecsToNextInvocation = mcp9808_idleTime;
		} else {
			mcp9808_returnStructure.sensorState = mcp9808;
			mcp9808_returnStructure.usecsToNextInvocation = mcp9808_temperatureMeasurementTime;
		}
	}
	return &mcp9808_returnStructure;
}

const char *mcp9808_getActionName(gpointer mcp9808SensorStat) {
	struct mcp9808_sensorStat *mcp9808 = (struct mcp9808_sensorStat *)mcp9808SensorStat;
	return mcp9808->sensorStateName;
}

struct inputNotifications_t *mcp9808_actionStateWatchedInputs() {
	return &noInputsToWatch;
}

struct allSensorsDescription_t *mcp9808_actionStateAllSensors(gpointer mcp9808SensorStat) {
	struct mcp9808_sensorStat *mcp9808 = (struct mcp9808_sensorStat *)mcp9808SensorStat;
	return mcp9808->allSensors;
}


void mcp9808_closeActionFunction(gpointer mcp9808SensorStatus) {
	struct mcp9808_sensorStat *mcp9808 = (struct mcp9808_sensorStat *)mcp9808SensorStatus;
	mcp9808_destroyTemperatureSensorState(mcp9808->device);
}


struct actionDescriptorStructure_t mcp9808ActionStructure = {
	.sensorType             = "MCP9808",
	.sensorStatePtr         = NULL,
	.initiateActionFunction = &mcp9808_initActionFunction,
	.stateWatchedInputs     = &mcp9808_actionStateWatchedInputs,
	.stateAllSensors        = &mcp9808_actionStateAllSensors,
	.actionFunction         = &mcp9808_actionFunction,
	.getActionNameFunction  = &mcp9808_getActionName,
	.destroyActionFunction  = &mcp9808_closeActionFunction
};
