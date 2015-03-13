#include "actionQueue.h"
#include "h21df_action.h"
#include "h21df_library.h"
#include <syslog.h>

const long h21_measurement_offset = 15 * 1000 * 1000; //15 secs

const char* h21dfTemperatureSensorName = "H21DF-Temperature";
const char* h21dfHumiditySensorName = "H21DF-Humidity";
const char* h21dfSensorStateName = "H21DF";


long h21df_initActionFunction(GHashTable *sensorStatus) {

	struct h21dfDevice *dev = h21DF_init(1);
	if (dev != NULL) {
		g_hash_table_replace(sensorStatus, (gpointer) h21dfSensorStateName, dev);
		return 10000;
	} else {
		syslog(LOG_ERR, "Unable to initiate h21df humidity sensor.");
		return -1;
	}
}

long h21df_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus) {
	struct h21dfDevice *dev = (struct h21dfDevice *)g_hash_table_lookup(sensorStatus, h21dfSensorStateName);

	if (dev == NULL) {
		syslog(LOG_ERR, "H21DF sensor status was not found. Can not continue.");
		return -1;
	};

	double temperature = h21DF_readTemperature(dev);
	double humidity = h21DF_readHumidity(dev);

	struct actionOutputItem *ao_temperature = (struct actionOutputItem *) malloc(sizeof(actionOutputItem));
	struct actionOutputItem *ao_humidity    = (struct actionOutputItem *) malloc(sizeof(actionOutputItem));

	ao_temperature->sensorDisplayName = h21dfTemperatureSensorName;
	ao_temperature->timeValueMeasured = getCurrentUSecs();
	ao_temperature->sensorValue       = temperature;

	ao_humidity->sensorDisplayName = h21dfHumiditySensorName;
	ao_humidity->timeValueMeasured = getCurrentUSecs();
	ao_humidity->sensorValue       = humidity;

	g_hash_table_replace(measurementOutput, (gpointer) h21dfTemperatureSensorName, ao_temperature);
	g_hash_table_replace(measurementOutput, (gpointer) h21dfHumiditySensorName, ao_humidity);

	return h21_measurement_offset;
}

void h21df_closeActionFunction(GHashTable *sensorStatus) {
	struct h21dfDevice *dev = (struct h21dfDevice *)g_hash_table_lookup(sensorStatus, h21dfSensorStateName);

	if (dev == NULL) {
		syslog(LOG_ERR, "H21DF sensor status was not found. Can not continue.");
		return;
	};
	h21DF_close(dev);

	g_hash_table_remove(sensorStatus, (gpointer) h21dfSensorStateName);
}
