#ifndef __lagon_sensor_config_h__
#define __lagon_sensor_config_h__

#include <glib.h>

struct sensorConfig_t {
	char *sensorType;
	char *sensorNameAppendix;
	char *sensorAddress;
	char *sensorOptions;
} sensorConfig_t;

GList *readConfigurationFile(const char* filename);
void freeSingleSensorConfiguration(gpointer ptr);

#endif