#include "tsl25911_luminosity.h"
#include <math.h>

const uint8_t tsl25911EnableRegister = 0x00;

struct tsl25911State *tsl25911_initLuminositySensor(uint8_t i2cBusID, uint8_t tsl25911Address) {
	int fd = i2c_initDevice(i2cBusID);
	if (fd < 0) {
		char *errorMsg = (char *) malloc(1024);
		snprintf(errorMsg, 1024, "Error opening I2C connection to TSL25911 luminosity sensor. Message: %s\n", strerror(errno));
		syslog(LOG_ERR, errorMsg);
		free(errorMsg);
		return NULL;
	}
	struct tsl25911State *tsl25911 = (struct tsl25911State*) malloc(sizeof(struct tsl25911State));
	tsl25911->address = mcp9808Address;
	tsl25911->i2cBusDevice = fd;

	if (mcp9808_initialize(tsl25911) == NULL) {
		i2c_closeDevice(fd);
		free(tsl25911);
		return NULL;
	} else {
		return tsl25911;
	}
}

uint8_t constructBusAddress(uint8_t registerAddr) {
	registerAddr = registerAddr & 0x1F;
	return (0x80 | 0x20 | registerAddr);
}

void tsl25911_enableMeasurement(struct tsl25911State *tsl25911) {
	i2c_write8bits(tsl25911->i2cBusDevice, tsl25911->address, constructBusAddress(tsl25911EnableRegister), 0x03); //0000 0011
}

void tsl25911_disableMeasurement(struct tsl25911State *tsl25911) {
	i2c_write8bits(tsl25911->i2cBusDevice, tsl25911->address, constructBusAddress(tsl25911EnableRegister), 0x00); //0000 0000
}
void tsl25911_setConfiguration(struct tsl25911State *tsl25911);
 tsl25911_getDeviceStatus(struct tsl25911State *tsl25911);

uint8_t tsl25911_verifyLuminositySensorPresent(struct tsl25911State *tsl25911);

void tsl25911_destroyTemperatureSensorState(struct tsl25911State *tsl25911);

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