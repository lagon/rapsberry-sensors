#include "printSensorsStat_action.h"
#include <stdio.h>
#include <malloc.h>
#include <syslog.h>

const char *printSensorStatusName = "PrintOutStatus";
const long printSensorInterval = 15 * 1000 * 1000; //5 secs

long print_initActionFunction(GHashTable *sensorStatus) {
	struct printActionStatus *stat = (struct printActionStatus *)malloc(sizeof(struct printActionStatus));
	stat->last_printout = getCurrentUSecs();
	g_hash_table_replace(sensorStatus, (gpointer) printSensorStatusName, stat);
	return 10000;
}

void printSensorFnc(gpointer key, gpointer value, gpointer userData) {
	struct actionOutputItem *v = (struct actionOutputItem *) value;
	printf("Sensor %s has value of %.1f\n", v->sensorDisplayName, v->sensorValue);
}

long print_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus) {
	g_hash_table_foreach(measurementOutput, &printSensorFnc, NULL);
	return printSensorInterval;
}


void print_closeActionFunction(GHashTable *sensorStatus) {
	struct printActionStatus *status = (struct printActionStatus *)g_hash_table_lookup(sensorStatus, printSensorStatusName);

	if (status == NULL) {
		syslog(LOG_ERR, "Print to console sensor status was not found. Can not continue.");
		return;
	};
	
	free(status);

	g_hash_table_remove(sensorStatus, (gpointer) printSensorStatusName);
}
