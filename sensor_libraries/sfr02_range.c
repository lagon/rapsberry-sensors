#include <sfr02_range.h>

const uint8_t sfr02_commandRegister = 0x00;
const uint8_t sfr02_rangeHighRegister = 0x02;

const uint8_t sfr02_measureInch = 0x50;
const uint8_t sfr02_measureCm   = 0x51;
const uint8_t sfr02_measureUSec = 0x52;


struct sfr02Device *sfr02_initDevice(int bus_id, uint8_t busAddr) {
	int fd = i2c_initDevice(bus_id);
	if (fd < 0) {
		return NULL;
	}
	struct sfr02Device* dev = (struct sfr02Device *) malloc(sizeof(struct sfr02Device));

	dev->bus_fd = fd;
	dev->address = busAddr;
	sfr02_setMeasurementUnits(dev, SFR02_CM);
	return dev;
}

int sfr02_setMeasurementUnits(struct sfr02Device *device, sfr02_units_reading_t units) {
	device->units = units;
}

sfr02_units_reading_t sfr02_getMeasurementUnits(struct sfr02Device *device) {
	return device->units;
}

int sfr02_initiateReading(struct sfr02Device *device) {
	uint8_t measurementCode;
	switch(device->units) {
		case SFR02_INCH: measurementCode = sfr02_measureInch; break;
		case SFR02_CM:   measurementCode = sfr02_measureCm;   break;
		case SFR02_USEC: measurementCode = sfr02_measureUSec; break;
		default: return -100;
	}
	return i2c_write8bits(device->bus_fd, device->address, sfr02_commandRegister, measurementCode);
}

int sfr02_LastReadingValue(struct sfr02Device *device, uint16_t *lastValue) {
	int ret = i2c_read16bits(device->bus_fd, device->address, sfr02_rangeHighRegister, lastValue);
	*lastValue = ((*lastValue & 0x00FF) << 8) + (*lastValue >> 8);
	return ret;
}

int sfr02_closeDevice(struct sfr02Device *device) {
	i2c_closeDevice(device->bus_fd);
	free(device);
	return 1;
}

int sfr02_testRanging() {
	struct sfr02Device *dev = sfr02_initDevice(1, 0x70);
	uint16_t dist = 0;
	while (1) {
		sfr02_initiateReading(dev);
		usleep(100000);
		sfr02_LastReadingValue(dev, &dist);
		printf("Reported distance: %5d cm\n", dist);
		usleep(100000);
	}
}
