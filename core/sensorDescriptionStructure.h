#ifndef __sensor_description_structure__
#define __sensor_description_structure__

struct singleSensorDescription_t {
	char *sensorID;
	char *sensorDisplayName;
	char *sensorUnits;
	char *sensorValueName;
} singleSensorDescription_t;

struct allSensorsDescription_t {
	unsigned int numSensors;
	struct singleSensorDescription_t sensorDescriptions[];
} allSensorsDescription_t;

#endif