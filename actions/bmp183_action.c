#include "actionQueue.h"
#include "bmp183_action.h"
#include "bmp183_library.h"
#include <syslog.h>

const char* bmp183PressureSensorName = "BMP183-Pressure";
const char* bmp183PressureSensorStateName = "BMP183";
const long long bmp183_PressureSensorRefresh = 15 * 1000 * 1000; //15 secs

enum bmp183_measurementState {BMP183_NOP, BMP183_MEASURING_TEMP, BMP183_MEASURING_PRESSURE} bmp183_measurementState;

struct bmp183_sensorStat {
	struct bmp183_device *device;
	enum bmp183_measurementState state;
} bmp183_sensorStat;

long bmp183_initActionFunction(GHashTable *sensorStatus) {
	struct bmp183_sensorStat *status = (struct bmp183_sensorStat *) malloc(sizeof(struct bmp183_device));
	status->device = bmp183_init(0, 0, 1000000, BMP183_ULTRA_HIGH_RESOLUTION);
	if (status->device == NULL) {
		syslog(LOG_ERR, "Initiation of BMP183 pressure sensor failed.");
		free(status);
		return -1;
	}
	status->state = BMP183_NOP;
	g_hash_table_replace(sensorStatus, (gpointer) bmp183PressureSensorStateName, status);
	return 10000;
}

long bmp183_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus) {
	struct bmp183_sensorStat *status = (struct bmp183_sensorStat *)g_hash_table_lookup(sensorStatus, bmp183PressureSensorStateName);

	switch (status->state) {
		case BMP183_NOP:
			status->state = BMP183_MEASURING_TEMP;
			bmp183_initiateTemeratureMeasurement(status->device);
			break;
		case BMP183_MEASURING_TEMP:
			status->state = BMP183_MEASURING_PRESSURE;
			double temp = bmp183_readTemperature(status->device);
			printf("Pressure sensor temperature %.2fC\n", temp);
			bmp183_initiatePressureMeasurement(status->device);
			break;
		case BMP183_MEASURING_PRESSURE:
			status->state = BMP183_NOP;
			double pressure = bmp183_readPressure(status->device);
			printf("Current Athmospheric Pressure %.2fPa\n", pressure);

			struct actionOutputItem *ao_pressure = (struct actionOutputItem *)malloc(sizeof(struct actionOutputItem));
			ao_pressure->sensorDisplayName = bmp183PressureSensorName;
			ao_pressure->timeValueMeasured = getCurrentUSecs();
			ao_pressure->sensorValue = pressure;

			g_hash_table_replace(measurementOutput, (gpointer) bmp183PressureSensorName, ao_pressure);
			break;
	}

	switch (status->state) {
		case BMP183_NOP:
			return bmp183_PressureSensorRefresh;
		case BMP183_MEASURING_TEMP:
			return(bmp183_getMeasurementUSecs(status->device, BMP183_MEASURE_TEMPERATURE));
		case BMP183_MEASURING_PRESSURE:
			return(bmp183_getMeasurementUSecs(status->device, BMP183_MEASURE_PRESSURE));
	}
	return -1;
}

void bmp183_closeActionFunction(GHashTable *sensorStatus) {
	struct bmp183_sensorStat *status = (struct bmp183_sensorStat *)g_hash_table_lookup(sensorStatus, bmp183PressureSensorStateName);
	bmp183_close(status->device);

	g_hash_table_remove(sensorStatus, (gpointer) bmp183PressureSensorStateName);
}
