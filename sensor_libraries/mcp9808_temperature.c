#include "mcp9808_temperature.h"

const double mcp9808_critical_temperature = 50.0;
const double mcp9808_upper_alert_boundary = 35.0;
const double mcp9808_lower_alert_boundary = 15.0;

const uint8_t mcp9808ConfigurationRegister = 0x01;
const uint8_t mcp9808UpperLimitTemperatureRegister = 0x02;
const uint8_t mcp9808LowerLimitTemperatureRegister = 0x03;
const uint8_t mcp9808CriticalLimitTemperatureRegister = 0x04;
const uint8_t mcp9808AmbientTemperatureRegister = 0x05;
const uint8_t mcp9808ResolutionRegister = 0x05;

uint16_t convertDoubleToIntTemp(double temperature) {
	uint8_t sign = 0;
	if (temperature < 0) {
		sign = 1;
		temperature = -1 * temperature;
	}

	uint16_t raw = (uint16_t) (temperature * 16);
	raw = 0x0FFC & raw;
	raw = raw | (sign << 12);
	return raw;
}

void mcp9808_initialize(struct mcp9808State *mcp9808) {
	mcp9808->configuration = 0x0100;
	uint16_t configuration = mcp9808->configuration; //0000 0011 0000 0000 = 1.5C hysteresis & shutdown
	uint16_t t_crit  = convertDoubleToIntTemp(mcp9808_critical_temperature);
	uint16_t t_upper = convertDoubleToIntTemp(mcp9808_upper_alert_boundary);
	uint16_t t_lower = convertDoubleToIntTemp(mcp9808_lower_alert_boundary);
	uint8_t resolution = 0x03;

	i2c_write16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808ConfigurationRegister, configuration);
	i2c_write16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808CriticalLimitTemperatureRegister, t_crit);
	i2c_write16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808UpperLimitTemperatureRegister, t_upper);
	i2c_write16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808LowerLimitTemperatureRegister, t_lower);
	i2c_write8bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808ResolutionRegister, resolution);
}

struct mcp9808State *mcp9808_initTemperatureSensor(uint8_t i2cBusID, uint8_t mcp9808Address) {
	int fd = i2c_initDevice(i2cBusID);
	if (fd < 0) {
		char *errorMsg = (char *) malloc(1024);
		snprintf(errorMsg, 1024, "Error opening I2C connection to MPV9808 temperature sensor. Message: %s\n", strerror(errno));
		syslog(LOG_ERR, errorMsg);
		free(errorMsg);
		return NULL;
	}
	struct mcp9808State *mcp9808 = (struct mcp9808State*) malloc(sizeof(struct mcp9808State));
	mcp9808->address = mcp9808Address;
	mcp9808->i2cBusDevice = fd;

	mcp9808_initialize(mcp9808);

	return mcp9808;
}

void mcp9808_stopMeasuring(struct mcp9808State *mcp9808) {
	uint16_t configuration = mcp9808->configuration; //0000 0011 0000 0000 = 1.5C hysteresis & shutdown
	configuration = configuration & 0xFEFF; // Make sure shutdown bit is 0;
	i2c_write16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808ConfigurationRegister, configuration);
}

void mcp9808_startMeasuring(struct mcp9808State *mcp9808) {
	uint16_t configuration = mcp9808->configuration; //0000 0011 0000 0000 = 1.5C hysteresis & NO shutdown
	configuration = configuration | 0x0100; // Make sure shutdown bit is 1;
	i2c_write16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808ConfigurationRegister, configuration);
}

double mcp9880_readTemperature(struct mcp9808State *mcp9808) {
	uint16_t raw = i2c_read16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808AmbientTemperatureRegister);
	raw = raw & 0x1FFF;
	uint8_t sign = (raw & 0x1000) >> 12;
	double temperature = ((double)(raw & 0x0FFF)) / 16.0;
	if (sign == 1) {
		temperature = -1 * temperature;
	}
	return temperature;
}

int mcp9808_isAboveCritical(struct mcp9808State *mcp9808) {
	uint16_t raw = i2c_read16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808AmbientTemperatureRegister);
	return (raw & 0x8000) == 0x8000;
}

int mcp9808_isAboveUpperLimit(struct mcp9808State *mcp9808) {
	uint16_t raw = i2c_read16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808AmbientTemperatureRegister);
	return (raw & 0x4000) == 0x4000;
}

int mcp9808_isBelowLowerLimit(struct mcp9808State *mcp9808) {
	uint16_t raw = i2c_read16bits(mcp9808->i2cBusDevice, mcp9808->address, mcp9808AmbientTemperatureRegister);
	return (raw & 0x2000) == 0x2000;
}


void mcp9808_destroyTemperatureSensorState(struct mcp9808State *mcp9808) {
	i2c_closeDevice(mcp9808->i2cBusDevice);
	free(mcp9808);
}

void mcp9808_testMeasurement() {
	struct mcp9808State *state = mcp9808_initTemperatureSensor(1, 0x18);
	mcp9808_startMeasuring(state);
	sleep(1);
	printf("Temperature is : %f C\n", mcp9880_readTemperature(state));
	sleep(1);
	printf("Temperature is : %f C\n", mcp9880_readTemperature(state));
	sleep(1);
	printf("Temperature is : %f C\n", mcp9880_readTemperature(state));

}