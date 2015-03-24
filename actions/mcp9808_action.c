#include "actionQueue.h"
#include "mcp9808_action.h"

const char* mcp9808TemperatureSensorName = "MCP9808 High Accuracy Temperature Sensor";
const char* mcp9808TemperatureSensorStateName = "MCP9808";

const char *mcp9808TemperatureMeasurementName = "MCP9808 Temperature";
const char *mcp9808TemperatureName = "MCP9808 Temperature";

const long long mcp9808_temperatureMeasurementTime = 300 * 1000; //300ms
const long long mcp9808_idleTime = 60 * 1000 * 1000; //60 sec
const uint8_t measurementsInBurst = 10;

enum mcp9808_measurementState {MCP9808_IDLE, MCP9808_MEASURING} mcp9808_measurementState;

struct mcp9808_sensorStat {
	struct mcp9808State *device;
	int remainingMeasurementsInBurst;
	double totalTemperature;
	double temperature;
	enum mcp9808_measurementState state;
} ina219_sensorStat;


long mcp9808_initActionFunction(GHashTable *sensorStatus) {
	struct mcp9808_sensorStat *sensor = (struct mcp9808_sensorStat *) malloc(sizeof(struct mcp9808_sensorStat));
	sensor->device = mcp9808_initTemperatureSensor(1, 0x18);
	mcp9808_stopMeasuring(sensor->device);
	sensor->state = MCP9808_IDLE;
	g_hash_table_replace(sensorStatus, (gpointer) mcp9808TemperatureSensorStateName, sensor);
	return 20000;
}

long mcp9808_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus) {
	struct mcp9808_sensorStat *mcp9808 = (struct mcp9808_sensorStat *)g_hash_table_lookup(sensorStatus, mcp9808TemperatureSensorStateName);

	if (mcp9808 == NULL) {
		syslog(LOG_ERR, "MCP9808 sensor status was not found. Can not continue.");
		return -1;
	};

	if (mcp9808->state == MCP9808_IDLE) {
		mcp9808_startMeasuring(mcp9808->device);
		mcp9808->totalTemperature = 0;
		mcp9808->remainingMeasurementsInBurst = measurementsInBurst;
		mcp9808->state = MCP9808_MEASURING;
		return mcp9808_temperatureMeasurementTime;
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

			struct actionOutputItem *ao_temp = generateOutputItem(mcp9808TemperatureName, mcp9808->temperature);
			g_hash_table_replace(measurementOutput, (gpointer) mcp9808TemperatureMeasurementName, ao_temp);
			return mcp9808_idleTime;
		} else {
			return mcp9808_temperatureMeasurementTime;
		}
	}
}


void mcp9808_closeActionFunction(GHashTable *sensorStatus) {
	struct mcp9808_sensorStat *mcp9808 = (struct mcp9808_sensorStat *)g_hash_table_lookup(sensorStatus, mcp9808TemperatureSensorStateName);
	mcp9808_destroyTemperatureSensorState(mcp9808->device);

	g_hash_table_remove(sensorStatus, (gpointer) mcp9808TemperatureSensorStateName);

}

