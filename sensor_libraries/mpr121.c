#include "mpr121.h"

#include <utilityFunctions.h>
#include <i2clib.h>

const uint8_t touchStatusRegister_0_7_address       = 0x00;
const uint8_t touchStatusRegister_8_12_address      = 0x01;
const uint8_t outOfRangeStatusRegister_0_7_address  = 0x02;
const uint8_t outOfRangeStatusRegister_8_12_address = 0x03;
const uint8_t filteredElectrodeData_LSB_BaseAddress = 0x04;
const uint8_t filteredElectrodeData_MSB_BaseAddress = 0x05;

const uint8_t electrodeBaselineValue_BaseAddress    = 0x1E;

const uint8_t mhdRising_address                     = 0x2B;
const uint8_t nhdAmountRising_address               = 0x2C;
const uint8_t nclRising_address                     = 0x2D;
const uint8_t fdlRising_address                     = 0x2E;
const uint8_t mhdFalling_address                    = 0x2F;
const uint8_t ndhAmountFalling_address              = 0x30;
const uint8_t nclFalling_address                    = 0x31;
const uint8_t fdlFalling_address                    = 0x32;
const uint8_t nhdAmountTouched_address              = 0x33;
const uint8_t nclTouched_address                    = 0x34;
const uint8_t fdlTouched_address                    = 0x35;

const uint8_t eleproxMhdRising_address              = 0x36;
const uint8_t eleproxNdhAmountRising_address        = 0x37;
const uint8_t eleproxNclRising_address              = 0x38;
const uint8_t eleproxFdlRising_address              = 0x39;
const uint8_t eleproxMhdFalling_address             = 0x3A;
const uint8_t eleproxNdhAmountFalling_address       = 0x3B;
const uint8_t eleproxNclFalling_address             = 0x3C;
const uint8_t eleproxFdlFalling_address             = 0x3D;
const uint8_t eleproxNhdAmountTouched_address       = 0x3E;
const uint8_t eleproxNclTouched_address             = 0x3F;
const uint8_t eleproxFdlTouched_address             = 0x40;

const uint8_t touchThreshold_BaseAddress            = 0x41;
const uint8_t releaseThreshold_BaseAddress          = 0x42;
const uint8_t debounceTouchAndRelease_address       = 0x5B;
const uint8_t globalCdcConfiguration_address        = 0x5C;
const uint8_t globalCdtConfiguration_address        = 0x5D;
const uint8_t electrodeConfiguration_address        = 0x5E;
const uint8_t electrodeCurrent_BaseAddress          = 0x5F;
const uint8_t electrodeChargeTime_BaseAddress       = 0x6C;
const uint8_t gpioControlRegister0_address          = 0x73;
const uint8_t gpioControlRegister1_address          = 0x74;
const uint8_t gpioDataRegister_address              = 0x75;
const uint8_t gpioDirectionRegister_address         = 0x76;
const uint8_t gpioEnableRegister_address            = 0x77;
const uint8_t gpioDataSetRegister_address           = 0x78;
const uint8_t gpioDataClearRegister_address         = 0x79;
const uint8_t gpioDataToggleRegister_address        = 0x7A;

const uint8_t autoConfigControlRegister0_address    = 0x7B;
const uint8_t autoConfigControlRegister1_address    = 0x7C;
const uint8_t autoConfigUslRegister_address         = 0x7D;
const uint8_t autoConfigLslRegister_address         = 0x7E;
const uint8_t autoConfigTargetLevelRegister_address = 0x7F;
const uint8_t softResetRegister_address             = 0x80;

void _mpr121_refreshTouchStatus(struct mpr121_device *dev) {
	printf("Refreshing...\n");
	if (dev->isRunningMode == 0) { //Not running at all
		dev->dataValid = 0;
		return;
	}

	uint8_t lowerSensors, upperSensors;

	i2c_read8bits(dev->bus_fd, dev->address, touchStatusRegister_0_7_address, &lowerSensors);
	i2c_read8bits(dev->bus_fd, dev->address, touchStatusRegister_8_12_address, &upperSensors);

	if ((upperSensors & 0x80) == 0) {
		dev->dataValid = 1;
		dev->isRunningMode = 1;
	} else {
		dev->dataValid = 0;
		dev->isRunningMode = 0;
	}
	dev->touchStatus = (upperSensors << 8) + lowerSensors;
	printf("Refreshed to: %0X - from %X, %X\n", dev->touchStatus, upperSensors, lowerSensors);
	dev->lastRefresh = getCurrentUSecs();
}

// -1 = Error, 0 = No, 1 = Yes
int mpr121_isElectrodeTouched(struct mpr121_device *dev, uint8_t electrodeID) {
	if (electrodeID > 13) {
		return -1;
	}
	if (dev->isRunningMode == 0) {
		return -3;
	}
	if (getCurrentUSecs() - dev->lastRefresh > dev->measurementRefreshIntervalUsec) {
		_mpr121_refreshTouchStatus(dev);
	}
	if (dev->dataValid == 0) {
		return -2;
	}
	if ((dev->touchStatus & (0x01 << electrodeID)) != 0) {
		return 1;
	} else {
		return 0;
	}

}

uint16_t mpr121_getElectrodeFilteredValues(struct mpr121_device *dev, uint8_t electrodeID) {
	if (electrodeID > 13) {
		return 0xFFFF;
	}
	uint8_t addr = filteredElectrodeData_LSB_BaseAddress + 2 * electrodeID;
	uint16_t value;
	i2c_read16bits(dev->bus_fd, dev->address, addr, &value);
	return value;
}

uint16_t mpr121_getElectrodeBaseLineValue(struct mpr121_device *dev, uint8_t electrodeID) {
	if (electrodeID > 13) {
		return 0xFFFF;
	}
	uint8_t addr = filteredElectrodeData_LSB_BaseAddress + electrodeID;
	uint8_t value;
	i2c_read8bits(dev->bus_fd, dev->address, addr, &value);
	uint16_t value16 = value << 2;
	return value16;
}

int mpr121_SetTouchThreshold(struct mpr121_device *dev, uint8_t electrodeID, uint8_t threshold) {
	if (electrodeID > 12) {
		return -1;
	}
	uint8_t addr = electrodeID * 2 + touchThreshold_BaseAddress;
	if (i2c_write8bits(dev->bus_fd, dev->address, addr, threshold) < 0) {
		return -1;
	} else {
		return 1;
	}
}

int mpr121_SetReleaseThreshold(struct mpr121_device *dev, uint8_t electrodeID, uint8_t threshold) {
	if (electrodeID > 12) {
		return -1;
	}
	uint8_t addr = electrodeID * 2 + releaseThreshold_BaseAddress;
	if (i2c_write8bits(dev->bus_fd, dev->address, addr, threshold) < 0) {
		return -1;
	} else {
		return 1;
	}
}


int mpr121_resetAndSetup(struct mpr121_device *dev) {
//Send Reset
	if (i2c_write8bits(dev->bus_fd, dev->address, softResetRegister_address, 0x63) < 0) return -1; // Reset

	usleep(10000);

	mpr121_putToStopMode(dev);

	usleep(10000);

	uint8_t cfgRead;
	i2c_read8bits(dev->bus_fd, dev->address, globalCdtConfiguration_address, &cfgRead);
	if (cfgRead != 0x24) {
		printf("The MPR121 did not respond properly. Should say 0x24 instead of %X. Stopping.\n", cfgRead);
		return -10;
	}


//Set GPIO
	if (i2c_write8bits(dev->bus_fd, dev->address, gpioControlRegister0_address, 0x00) < 0) return -1; 
	if (i2c_write8bits(dev->bus_fd, dev->address, gpioControlRegister1_address, 0x00) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, gpioDirectionRegister_address, 0x00) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, gpioEnableRegister_address, 0xFF) < 0) return -1;

	if (i2c_write8bits(dev->bus_fd, dev->address, mhdRising_address, 0x01) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, nhdAmountRising_address, 0x01) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, nclRising_address, 0x00) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, fdlRising_address, 0x00) < 0) return -1;

	if (i2c_write8bits(dev->bus_fd, dev->address, mhdFalling_address, 0x01) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, ndhAmountFalling_address, 0x01) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, nclFalling_address, 0xFF) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, fdlFalling_address, 0x02) < 0) return -1;

	if (i2c_write8bits(dev->bus_fd, dev->address, eleproxMhdRising_address, 0x01) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, eleproxNdhAmountRising_address, 0x01) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, eleproxNclRising_address, 0x00) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, eleproxFdlRising_address, 0x00) < 0) return -1;

	if (i2c_write8bits(dev->bus_fd, dev->address, eleproxMhdFalling_address, 0x01) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, eleproxNdhAmountFalling_address, 0x01) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, eleproxNclFalling_address, 0xFF) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, eleproxFdlFalling_address, 0x02) < 0) return -1;

	for (int electrodeID = 0; electrodeID < 13; electrodeID++) {
		if(mpr121_SetTouchThreshold(dev, electrodeID, 0x06) < 0) return -1;
		if(mpr121_SetReleaseThreshold(dev, electrodeID, 0x0A) < 0) return -1;
	}


	if (i2c_write8bits(dev->bus_fd, dev->address, globalCdcConfiguration_address, 0x90) < 0) return -1; // 1001 0000
	if (i2c_write8bits(dev->bus_fd, dev->address, globalCdtConfiguration_address, 0x3C) < 0) return -1; // 0011 1100

	if (i2c_write8bits(dev->bus_fd, dev->address, debounceTouchAndRelease_address, 0x11) < 0) return -1;

	if (i2c_write8bits(dev->bus_fd, dev->address, autoConfigLslRegister_address, 0x00) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, autoConfigUslRegister_address, 0x00) < 0) return -1;
	if (i2c_write8bits(dev->bus_fd, dev->address, autoConfigTargetLevelRegister_address, 0x00) < 0) return -1;

	if (i2c_write8bits(dev->bus_fd, dev->address, autoConfigControlRegister1_address, 0x00) < 0) return -1; // 0XXX 0000
	if (i2c_write8bits(dev->bus_fd, dev->address, autoConfigControlRegister0_address, 0xB3) < 0) return -1; // 1011 0011

	return 1;
}

int mpr121_isAutoConfigurastionDone(struct mpr121_device *dev) {
	uint8_t status;
	i2c_read8bits(dev->bus_fd, dev->address, outOfRangeStatusRegister_8_12_address, &status);
	if (status == 0xFF) {
		return -1;
	}
	if ((status & 0xC0) != 0) { //1100 0000 - autoconfiguration is not finished
		return 0;
	} else {
		return 1;
	}
}

int mpr121_putToStopMode(struct mpr121_device *dev) {
	dev->isRunningMode = 0;
	if (i2c_write8bits(dev->bus_fd, dev->address, electrodeConfiguration_address, 0x00) < 0) {
		return -1;
	} else {
		return 1;
	}
}

int mpr121_putToRunningMode(struct mpr121_device *dev) {
	int autoconfStatus = mpr121_isAutoConfigurastionDone(dev);
	if (autoconfStatus < 0) {
		return -1;
	} else if (autoconfStatus == 0) {
		dev->isRunningMode = 0;
		return 0;
	} else {
		//Switch to running mode
		uint8_t outData = 0xC0;
		outData = outData | ((dev->proximitySensorSize & 0x03) << 4);
		outData = outData | (dev->maxElectrodeToTrack & 0x0F);
		printf("%0X\n", outData);
		if (i2c_write8bits(dev->bus_fd, dev->address, electrodeConfiguration_address, outData) < 0) {
			return -1;
		} // 0011 1111
		dev->isRunningMode = 1;
		return 1;
	}
	return 0;
}


struct mpr121_device *mpr121_initializeWithAllElectrodesEnabled(int bus_id, uint8_t address) {
	int bus_fd = i2c_initDevice(bus_id);
	if (bus_fd < 0) {
		return NULL;
	}

	struct mpr121_device *dev = (struct mpr121_device *) malloc(sizeof(struct mpr121_device));
	if (dev == NULL) {
		return NULL;
	}

	dev->bus_fd = bus_fd;
	dev->address = address;

	dev->measurementRefreshIntervalUsec	= 200 * 1000;
	dev->lastRefresh = 0;
	dev->isRunningMode = 0;
	dev->maxElectrodeToTrack = 12;
	dev->proximitySensorSize = 3;

	if (mpr121_resetAndSetup(dev) < 0) {
		i2c_closeDevice(bus_fd);
		free(dev);
		return NULL;
	}

	usleep(dev->measurementRefreshIntervalUsec);

	if (mpr121_putToRunningMode(dev) < 0) {
		i2c_closeDevice(bus_fd);
		free(dev);
		return NULL;		
	}

	return dev;
}

void mpr121_finishAndClose(struct mpr121_device *dev) {
	mpr121_putToStopMode(dev);
	i2c_closeDevice(dev->bus_fd);
	free(dev);
}



