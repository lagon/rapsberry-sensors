#include "ina219_power_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

const uint8_t config_reagister_addr       = 0x00;
const uint8_t shunt_voltage_register_addr = 0x01;
const uint8_t bus_voltage_register_addr   = 0x02;
const uint8_t power_register_addr         = 0x03;
const uint8_t current_register_addr       = 0x04;
const uint8_t calibration_register_addr   = 0x05;
const double  shunt_resistor_value_ohms = 0.1;  

const uint16_t calibration_value = 8192;
//Calibration set to allow max 2A and 50uA per bit in current register and 800uW in power register.
//See github.com/adafruit/Adafruit_INA219/blob/master/Adafruit_INA219.cpp
const double current_LSB_mA = 0.05;
const double power_LSB_mW   = 20 * 0.05;


struct ina219State *ina219_initPowerMonitor(uint8_t i2cBusID, uint8_t ina219Address) {
	int fd = i2c_initDevice(i2cBusID);
	if (fd < 0) {
		char *errorMsg = (char *) malloc(1024);
		snprintf(errorMsg, 1024, "Error opening I2C connection to INA219 Power Monitor. Message: %s\n", strerror(errno));
		syslog(LOG_ERR, errorMsg);
		free(errorMsg);
		return NULL;
	}
	struct ina219State *ina219 = (struct ina219State*) malloc(sizeof(struct ina219State));
	ina219->address = ina219Address;
	ina219->i2cBusDevice = fd;
	return ina219;
}

void ina219_setCalibrationRegister(struct ina219State *ina219) {
	uint8_t data[3];
	data[0] = calibration_register_addr;
	data[1] = (calibration_value >> 8) & 0x00FF;
	data[2] = (calibration_value) & 0x00FF;
	i2c_writeToDevice(ina219->i2cBusDevice, ina219->address, data, 3);
}

void ina219_powerOn(struct ina219State *ina219) {
	// 1 - reset
	// 0 - NA
	// 1 - bus voltage range 32V
	// 11 - PGA Shunt voltage 320mV
	// 0011 - BADC 12b bus voltage resolution
	// 0011 - SADC 12b shunt voltage resolution
	// 100 - Powered on
	// 1011 1001 1001 1100 = 0xB9 0x9C
	uint8_t data[3] = {0x00, 0xB9, 0x9C};
	data[0] = config_reagister_addr;
	i2c_writeToDevice(ina219->i2cBusDevice, ina219->address, data, 3);
}

void ina219_powerOff(struct ina219State *ina219) {
	// 0 - reset
	// 0 - NA
	// 1 - bus voltage range 32V
	// 11 - PGA Shunt voltage 320mV
	// 0011 - BADC 12b bus voltage resolution
	// 0011 - SADC 12b shunt voltage resolution
	// 000 - Powered off
	// 0011 1001 1001 1000 = 0xB9 0x98
	uint8_t data[3] = {0x00, 0x39, 0x98};
	data[0] = config_reagister_addr;
	i2c_writeToDevice(ina219->i2cBusDevice, ina219->address, data, 3);
}

void ina219_initateVoltageReadingSingle(struct ina219State *ina219) {
	// 0 - reset
	// 0 - NA
	// 1 - bus voltage range 32V
	// 11 - PGA Shunt voltage 320mV
	// 0011 - BADC 12b bus voltage resolution
	// 0011 - SADC 12b shunt voltage resolution
	// 011 - Powered on & shunt + bus voltage
	// 0011 1001 1001 1011 = 0xB9 0x9B
	// 0011 1111 1111 1011 = 0xB9 0x9B
	uint8_t data[3] = {0x00, 0x3F, 0x9B};
	data[0] = config_reagister_addr;
	i2c_writeToDevice(ina219->i2cBusDevice, ina219->address, data, 3);
}

uint16_t ina219_read16bits(struct ina219State *ina219, uint8_t reg) {
	return i2c_read16bits(ina219->i2cBusDevice, ina219->address, reg);
}

int ina219_isReadingReady(struct ina219State *ina219) {
	uint16_t rawValue = ina219_read16bits(ina219, bus_voltage_register_addr);
	rawValue = rawValue >> 1;
	rawValue = 0x0001 & rawValue;
	//REading is valid when set to 1
	return rawValue;
}

int ina219_isReadingValid(struct ina219State *ina219) {
	uint16_t rawValue = ina219_read16bits(ina219, bus_voltage_register_addr);
	rawValue = 0x0001 & rawValue;
	//Reading is valid if bit is set to 0
	return (1 - rawValue);
}

double ina219_readBusVoltageSingle(struct ina219State *ina219) {
	uint16_t rawBusVoltage = ina219_read16bits(ina219, bus_voltage_register_addr);
	rawBusVoltage = rawBusVoltage >> 3;
	double busVoltage = ((double) rawBusVoltage) / 1000.0 * 4.0;
	return busVoltage;
}

double ina219_readShuntVoltageSingle(struct ina219State *ina219) {
	int16_t rawShuntVoltage = (int16_t) ina219_read16bits(ina219, shunt_voltage_register_addr);
	double shuntVoltage = ((double) rawShuntVoltage) * 0.01;
	return shuntVoltage;
}

double ina219_readPowerSingle(struct ina219State *ina219) {
	uint16_t rawPower = ina219_read16bits(ina219, power_register_addr);
	double power = ((double) rawPower) * power_LSB_mW;
	return power;
}

double ina219_readCurrentSingle(struct ina219State *ina219) {
	int16_t rawCurrent = (int16_t) ina219_read16bits(ina219, current_register_addr);
	double current = ((double) rawCurrent) * current_LSB_mA;
	return current;
}

void ina219_destroyPowerMonitorState(struct ina219State *ina219) {
	i2c_closeDevice(ina219->i2cBusDevice);
	free(ina219);
}

void ina219_testMeasurement() {
	struct ina219State *ina219 = ina219_initPowerMonitor(1, 0x44);
    ina219_powerOff(ina219);
    sleep(1);
    ina219_powerOn(ina219);
    sleep(1);
    ina219_setCalibrationRegister(ina219);  
    ina219_initateVoltageReadingSingle(ina219);
    printf("Is ready? %d\n", ina219_isReadingReady(ina219));
    sleep(1);
    printf("Is ready? %d\n", ina219_isReadingReady(ina219));
    printf("Is valid? %d\n", ina219_isReadingValid(ina219));
    printf("Bus voltage: %f V\n", ina219_readBusVoltageSingle(ina219));
    printf("Shunt voltage: %f mV\n", ina219_readShuntVoltageSingle(ina219));
    printf("Current: %f mA\n", ina219_readCurrentSingle(ina219));
    printf("Power: %f mW\n", ina219_readPowerSingle(ina219));
    ina219_destroyPowerMonitorState(ina219);
}

