#include "save2file_action.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <syslog.h>
#include <errno.h>

static char* filename = "./data/latestValues.csv";

const char *saveActualSensorStatusName = "SaveActualStatus";
const long saveActualSensorInterval = 60 * 1000 * 1000; //60 secs


long save_actual_initActionFunction(GHashTable *sensorStatus) {
	struct saveActualValueActionStatus *stat = (struct saveActualValueActionStatus *)malloc(sizeof(struct saveActualValueActionStatus));
	stat->filename = filename;
	g_hash_table_replace(sensorStatus, (gpointer) saveActualSensorStatusName, stat);
	return saveActualSensorInterval;
}

void log_error(char *action, char *msg) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	snprintf(str, 1024, "Unable to %s file with actual values. Reason: %s", action, strerror(errno));
	syslog(LOG_ERR, str);
	free(str);	
}

long save_actual_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus) {
	struct saveActualValueActionStatus *status = (struct saveActualValueActionStatus *)g_hash_table_lookup(sensorStatus, saveActualSensorStatusName);
	FILE *f = fopen(status->filename, "w");
	if (f == NULL) {
		log_error("open", strerror(errno));
		return saveActualSensorInterval;
	}

	GHashTableIter iterator;
	g_hash_table_iter_init(&iterator, measurementOutput);

	gpointer key;
	gpointer value;

	while (g_hash_table_iter_next (&iterator, &key, &value)) {
		struct actionOutputItem *stat = (struct actionOutputItem *) value;
    	if (fprintf(f, "%s\t%f\n", stat->sensorDisplayName, stat->sensorValue) < 0) {
    		log_error("write to", strerror(errno));
    		fclose(f);
    		return saveActualSensorInterval;
    	}
    }
    if (fclose(f) < 0) {
		log_error("close", strerror(errno));    	
    }
    return saveActualSensorInterval;
}

void save_actual_closeActionFunction(GHashTable *sensorStatus) {
	struct saveActualValueActionStatus *status = (struct saveActualValueActionStatus *)g_hash_table_lookup(sensorStatus, saveActualSensorStatusName);
	free(status->filename);
	g_hash_table_remove(sensorStatus, (gpointer) saveActualSensorStatusName);
}



