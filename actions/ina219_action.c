#include "actionQueue.h"
#include "ina219_action.h"
#include "ina219_power_monitor.h"
#include <syslog.h>

const char* ina219PowerMonitorSensorName = "INA219-PowerMonitor";
const char* ina219PowerMonitorSensorStateName = "INA219";
const long long ina219_powerMonitorRefresh = 999 * 1000; //0.999 sec
const long long ina219_measurementTime = 1000; //1ms

const char *ina219LastVoltageName        = "Last Voltage";
const char *ina219LastCurrentName        = "Last Current";
const char *ina219LastPowerName          = "Last Power";
const char *ina219TotalPowerStoredName   = "Total Power Stored";
const char *ina219TotelPowerConsumedName = "Total Power Consumed";

const char *ina219LastVoltageMeasurementName        = "INA219LastVoltageMeasurement";
const char *ina219LastCurrentMeasurementName        = "INA219LastCurrentMeasurement";
const char *ina219LastPowerMeasurementName          = "INA219LastPowerMeasurement";
const char *ina219TotalPowerStoredMeasurementName   = "INA219TotalPowerStoredMeasurement";
const char *ina219TotelPowerConsumedMeasurementName = "INA219TotalPowerConsumedMeasurement";

enum ina219_measurementState {INA219_NOP, INA219_MEASUREMENT_IN_PROGRESS} ina219_measurementState;

struct ina219_sensorStat {
	struct ina219State *device;
	double lastVoltage;
	double lastCurrent;
	double lastPower;
	double totalPowerConsumed;
	double totalPowerStored;
	enum ina219_measurementState measurementState;
} ina219_sensorStat;

long ina219_initActionFunction(GHashTable *sensorStatus) {
	struct ina219_sensorStat *sensor = (struct ina219_sensorStat *) malloc(sizeof(struct ina219_sensorStat));
	sensor->device = ina219_initPowerMonitor(1, 0x44);
	ina219_powerOn(sensor->device);
	ina219_setCalibrationRegister(sensor->device);  

	sensor->lastVoltage = 0;
	sensor->lastCurrent = 0;
	sensor->lastPower = 0;
	sensor->totalPowerConsumed = 0;
	sensor->totalPowerStored = 0;

	g_hash_table_replace(sensorStatus, (gpointer) ina219PowerMonitorSensorStateName, sensor);
	return 20000;
}

struct actionOutputItem *generateOutputItem(const char* name, double value) {
	struct actionOutputItem *ao_value = (struct actionOutputItem *) malloc(sizeof(struct actionOutputItem));
	ao_value->sensorDisplayName = name;
	ao_value->timeValueMeasured = getCurrentUSecs();
	ao_value->sensorValue       = value;
	return ao_value;
}

long ina219_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus) {
	struct ina219_sensorStat *ina219 = (struct ina219_sensorStat *)g_hash_table_lookup(sensorStatus, ina219PowerMonitorSensorStateName);

	if (ina219 == NULL) {
		syslog(LOG_ERR, "INA219 sensor status was not found. Can not continue.");
		return -1;
	};

	if (ina219->measurementState == INA219_NOP) {
		ina219_initateVoltageReadingSingle(ina219->device);
		ina219->measurementState = INA219_MEASUREMENT_IN_PROGRESS;
		return(ina219_measurementTime);
	} else {
		// if (ina219_isReadingReady(ina219->device) == 0) {
		// 	//Not ready yet
		// 	return ina219_measurementTime;
		// }
		ina219->lastVoltage = ina219_readBusVoltageSingle(ina219->device);
		ina219->lastCurrent = ina219_readCurrentSingle(ina219->device);
		ina219->lastPower   = ina219_readPowerSingle(ina219->device);
		if (ina219->lastPower > 0) {
			ina219->totalPowerConsumed = ina219->totalPowerConsumed + ina219->lastPower;
		} else {
			ina219->totalPowerStored = ina219->totalPowerStored + ina219->lastPower;
		}

		struct actionOutputItem *ao_lastVoltage        = generateOutputItem(ina219LastVoltageName,        ina219->lastVoltage);
		struct actionOutputItem *ao_lastCurrent        = generateOutputItem(ina219LastCurrentName,        ina219->lastCurrent);
		struct actionOutputItem *ao_lastPower          = generateOutputItem(ina219LastPowerName,          ina219->lastPower);
		struct actionOutputItem *ao_totalPowerStored   = generateOutputItem(ina219TotalPowerStoredName,   ina219->totalPowerStored);
		struct actionOutputItem *ao_totalPowerConsumed = generateOutputItem(ina219TotelPowerConsumedName, ina219->totalPowerConsumed);

		g_hash_table_replace(measurementOutput, (gpointer) ina219LastVoltageMeasurementName,        ao_lastVoltage);
		g_hash_table_replace(measurementOutput, (gpointer) ina219LastCurrentMeasurementName,        ao_lastCurrent);
		g_hash_table_replace(measurementOutput, (gpointer) ina219LastPowerMeasurementName,          ao_lastPower);
		g_hash_table_replace(measurementOutput, (gpointer) ina219TotalPowerStoredMeasurementName,   ao_totalPowerStored);
		g_hash_table_replace(measurementOutput, (gpointer) ina219TotelPowerConsumedMeasurementName, ao_totalPowerConsumed);
		ina219->measurementState = INA219_NOP;
		return ina219_powerMonitorRefresh;
	}
}

void ina219_closeActionFunction(GHashTable *sensorStatus) {}
