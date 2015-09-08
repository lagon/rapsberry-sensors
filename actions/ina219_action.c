// #include "actionQueue.h"
// #include "ina219_action.h"
// #include "ina219_power_monitor.h"
// #include <syslog.h>

// const char* ina219PowerMonitorSensorName = "INA219-PowerMonitor";
// const char* ina219PowerMonitorSensorStateName = "INA219";
// const long long ina219_powerMonitorRefresh = 9999 * 1000; //0.999 sec
// const long long ina219_measurementTime = 1000; //1ms

// const char *ina219LastVoltageName        = "Last Voltage";
// const char *ina219LastCurrentName        = "Last Current";
// const char *ina219LastPowerName          = "Last Power";
// const char *ina219TotalPowerStoredName   = "Total Power Stored";
// const char *ina219TotalPowerConsumedName = "Total Power Consumed";

// const char *ina219LastVoltageMeasurementName        = "INA219LastVoltageMeasurement";
// const char *ina219LastCurrentMeasurementName        = "INA219LastCurrentMeasurement";
// const char *ina219LastPowerMeasurementName          = "INA219LastPowerMeasurement";
// const char *ina219TotalPowerStoredMeasurementName   = "INA219TotalPowerStoredMeasurement";
// const char *ina219TotalPowerConsumedMeasurementName = "INA219TotalPowerConsumedMeasurement";

// enum ina219_measurementState {INA219_NOP, INA219_MEASUREMENT_IN_PROGRESS} ina219_measurementState;

// struct ina219_sensorStat {
// 	struct ina219State *device;
// 	double lastVoltage;
// 	double lastCurrent;
// 	double lastPower;
// 	double totalPowerConsumed;
// 	double totalPowerStored;
// 	enum ina219_measurementState measurementState;
// } ina219_sensorStat;

// struct allSensorsDescription_t ina219_allSensors = {
// 	.numSensors = 5,
// 	.sensorDescriptions = {{
// 			.sensorID = ina219LastVoltageMeasurementName, 
// 			.sensorDisplayName = "Voltage", 
// 			.sensorUnits = "V", 
// 			.sensorValueName = "Voltage"
// 		}, {
// 			.sensorID = ina219LastCurrentMeasurementName, 
// 			.sensorDisplayName = "Current",
// 			.sensorUnits = "A", 
// 			.sensorValueName = "Current"
// 		}, {
// 			.sensorID = ina219LastPowerMeasurementName, 
// 			.sensorDisplayName = "Power Currently Consumed",
// 			.sensorUnits = "W", 
// 			.sensorValueName = "Watts"
// 		}, {
// 			.sensorID = ina219TotalPowerStoredMeasurementName, 
// 			.sensorDisplayName = "Power Stored",
// 			.sensorUnits = "Wh", 
// 			.sensorValueName = "Watt Hours"
// 		}, {
// 			.sensorID = ina219TotalPowerConsumedMeasurementName, 
// 			.sensorDisplayName = "Power Consumed",
// 			.sensorUnits = "Wh", 
// 			.sensorValueName = "Watt Hours"
// 		}
// 	}
// };

// struct actionReturnValue_t ina219_returnStructure;

// struct actionReturnValue_t* ina219_initActionFunction() {
// 	struct ina219_sensorStat *sensor = (struct ina219_sensorStat *) malloc(sizeof(struct ina219_sensorStat));
// 	sensor->device = ina219_initPowerMonitor(1, 0x44);
// 	ina219_powerOn(sensor->device);
// 	ina219_setCalibrationRegister(sensor->device);  

// 	sensor->lastVoltage = 0;
// 	sensor->lastCurrent = 0;
// 	sensor->lastPower = 0;
// 	sensor->totalPowerConsumed = 0;
// 	sensor->totalPowerStored = 0;

// 	if (sensor->device == NULL) {
// 		ina219_returnStructure.sensorState = sensor;
// 		ina219_returnStructure.actionErrorStatus = -1;
// 		ina219_returnStructure.usecsToNextInvocation = -1;
// 		ina219_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
// 		ina219_returnStructure.changedInputs = generateNoInputsChanged();
// 	} else {
// 		ina219_returnStructure.sensorState = sensor;
// 		ina219_returnStructure.actionErrorStatus = 0;
// 		ina219_returnStructure.usecsToNextInvocation = ina219_powerMonitorRefresh;
// 		ina219_returnStructure.waitOnInputMode = WAIT_TIME_PERIOD;
// 		ina219_returnStructure.changedInputs = generateNoInputsChanged();
// 	}

// 	return &ina219_returnStructure;
// }

// long ina219_actionFunction(GHashTable* measurementOutput, GHashTable *sensorStatus) {
// 	struct ina219_sensorStat *ina219 = (struct ina219_sensorStat *)g_hash_table_lookup(sensorStatus, ina219PowerMonitorSensorStateName);

// 	if (ina219 == NULL) {
// 		syslog(LOG_ERR, "INA219 sensor status was not found. Can not continue.");
// 		return -1;
// 	};

// 	if (ina219->measurementState == INA219_NOP) {
// 		ina219_initateVoltageReadingSingle(ina219->device);
// 		ina219->measurementState = INA219_MEASUREMENT_IN_PROGRESS;
// 		return(ina219_measurementTime);
// 	} else {
// 		// if (ina219_isReadingReady(ina219->device) == 0) {
// 		// 	//Not ready yet
// 		// 	return ina219_measurementTime;
// 		// }
// 		ina219->lastVoltage = ina219_readBusVoltageSingle(ina219->device);
// 		ina219->lastCurrent = ina219_readCurrentSingle(ina219->device);
// 		ina219->lastPower   = ina219_readPowerSingle(ina219->device);
// 		if (ina219->lastPower > 0) {
// 			ina219->totalPowerConsumed = ina219->totalPowerConsumed + ina219->lastPower;
// 		} else {
// 			ina219->totalPowerStored = ina219->totalPowerStored + ina219->lastPower;
// 		}

// 		struct actionOutputItem *ao_lastVoltage        = generateOutputItem(ina219LastVoltageName,        ina219->lastVoltage);
// 		struct actionOutputItem *ao_lastCurrent        = generateOutputItem(ina219LastCurrentName,        ina219->lastCurrent);
// 		struct actionOutputItem *ao_lastPower          = generateOutputItem(ina219LastPowerName,          ina219->lastPower);
// 		struct actionOutputItem *ao_totalPowerStored   = generateOutputItem(ina219TotalPowerStoredName,   ina219->totalPowerStored);
// 		struct actionOutputItem *ao_totalPowerConsumed = generateOutputItem(ina219TotalPowerConsumedName, ina219->totalPowerConsumed);

// 		g_hash_table_replace(measurementOutput, (gpointer) ina219LastVoltageMeasurementName,        ao_lastVoltage);
// 		g_hash_table_replace(measurementOutput, (gpointer) ina219LastCurrentMeasurementName,        ao_lastCurrent);
// 		g_hash_table_replace(measurementOutput, (gpointer) ina219LastPowerMeasurementName,          ao_lastPower);
// 		g_hash_table_replace(measurementOutput, (gpointer) ina219TotalPowerStoredMeasurementName,   ao_totalPowerStored);
// 		g_hash_table_replace(measurementOutput, (gpointer) ina219TotalPowerConsumedMeasurementName, ao_totalPowerConsumed);
// 		ina219->measurementState = INA219_NOP;
// 		return ina219_powerMonitorRefresh;
// 	}
// }

// const char *ina219_getActionName() {
// 	return ina219PowerMonitorSensorName;
// }

// struct inputNotifications_t *h21df_actionStateWatchedInputs() {
// 	return &noInputsToWatch;
// }

// struct allSensorsDescription_t *h21df_actionStateAllSensors() {
// 	return &ina219_allSensors;
// }



// void ina219_closeActionFunction(GHashTable *sensorStatus) {
	
// }



// struct actionDescriptorStructure_t ina219ActionStructure = {
// 	.initiateActionFunction = &ina219_initActionFunction,
// 	.stateWatchedInputs     = &ina219_actionStateWatchedInputs,
// 	.stateAllSensors        = &ina219_actionStateAllSensors,
// 	.actionFunction         = &ina219_actionFunction,
// 	.getActionNameFunction  = &ina219_getActionName,
// 	.destroyActionFunction  = &ina219_closeActionFunction
// };


