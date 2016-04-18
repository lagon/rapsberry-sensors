#include <ht16k33.h>

const uint8_t HT16K33_SYSSETUP_CMD = 0x20;
const uint8_t HT16K33_BLINK_CMD = 0x80;
const uint8_t HT16K33_BRIGHTNESS_CMD = 0xE0;

struct ht16k33Device* ht16k33_initDevice(int bus_id, uint8_t address) {
	int fd = i2c_initDevice(bus_id);
	if (fd < 0) {
		return NULL;
	}
	struct ht16k33Device* dev = (struct ht16k33Device *) malloc(sizeof(struct ht16k33Device));

	dev->bus_fd = fd;
	dev->address = address;
	dev->buffer[0] = 0;

	dev->blinkState = HT16K33_NO_BLINK;
	dev->displayShowing = 0;
	dev->brightness = 0;

    return dev;
}

int ht16k33_powerOn(struct ht16k33Device *device) {
	uint8_t cmd = HT16K33_SYSSETUP_CMD | 0x01;
	return i2c_writeToDevice(device->bus_fd, device->address, &cmd, 1);
}

int ht16k33_powerOff(struct ht16k33Device *device) {
	uint8_t cmd = HT16K33_SYSSETUP_CMD;
	return i2c_writeToDevice(device->bus_fd, device->address, &cmd, 1);
}

int ht16k33_setBrightness(struct ht16k33Device *device, uint8_t brightnessLevel) {
	if (brightnessLevel > 15) {
		return -1;
	}
	device->brightness = brightnessLevel;
	uint8_t cmd = HT16K33_BRIGHTNESS_CMD | brightnessLevel;
	return i2c_writeToDevice(device->bus_fd, device->address, &cmd, 1);
}

int ht16k33_setDisplayRegister_internal(struct ht16k33Device *device) {
	uint8_t cmd = HT16K33_BLINK_CMD | device->displayShowing | (device->blinkState << 1);
	return i2c_writeToDevice(device->bus_fd, device->address, &cmd, 1);	
}

int ht16k33_setBlinking(struct ht16k33Device *device, blinkState_t blinkState) {
	device->blinkState = blinkState;
	return ht16k33_setDisplayRegister_internal(device);
}

int ht16k33_turnOn(struct ht16k33Device *device) {
	device->displayShowing = 1;
	return ht16k33_setDisplayRegister_internal(device);	
}

int ht16k33_turnOff(struct ht16k33Device *device) {
	device->displayShowing = 0;
	return ht16k33_setDisplayRegister_internal(device);	
}

int ht16k33_getBufferPosition(struct ht16k33Device *device, uint8_t position, uint16_t *value) {
	if (position >= (_ht16k33_bufferSize * 2)) {
		return -1;
	}
	*value = device->buffer[(2 * position) + 1];
	return 1;
}

int ht16k33_setBufferPosition(struct ht16k33Device *device, uint8_t position, uint16_t value) {
	if (position >= (_ht16k33_bufferSize * 2)) {
		return -1;
	}
	printf("%d == 0x%02X 0x%02X (was 0x%04X)\n", position, (uint8_t) ((value & 0xFF00) >> 8), (uint8_t) (value & 0x00FF), value);
	device->buffer[(2 * position) + 2] = (uint8_t) ((value & 0xFF00) >> 8);
	device->buffer[(2 * position) + 1] = (uint8_t) (value & 0x00FF);
	return 1;
}

int ht16k33_flushBufferToDisplay(struct ht16k33Device *device) {
	return i2c_writeToDevice(device->bus_fd, device->address, device->buffer, (_ht16k33_bufferSize * 2) + 1);
}

int ht16k33_closeDevice(struct ht16k33Device *device) {
	ht16k33_turnOff(device);
	close(device->bus_fd);
	free(device);
	return 1;
}

void ht16k33_testDisplay() {
	struct ht16k33Device* dev = ht16k33_initDevice(1, 0x70);
	ht16k33_powerOn(dev);
	ht16k33_turnOn(dev);
	ht16k33_setBlinking(dev, HT16K33_NO_BLINK);
	ht16k33_setBrightness(dev, 15);

	for (uint8_t pos = 0; pos < 5; pos++) {
		uint16_t x = 1;
		for	(uint8_t step = 0; step < 8; step++) {
			printf("Pos: %d, segment: %d\n", pos, step);
			ht16k33_setBufferPosition(dev, 0, 0);
			ht16k33_setBufferPosition(dev, 1, 0);
			ht16k33_setBufferPosition(dev, 2, 0);
			ht16k33_setBufferPosition(dev, 3, 0);
			ht16k33_setBufferPosition(dev, 4, 0);

			ht16k33_setBufferPosition(dev, pos, x << step);			
			ht16k33_flushBufferToDisplay(dev);
			usleep(500000);
		}
	}

	ht16k33_setBufferPosition(dev, 0, 0xFF);
	ht16k33_setBufferPosition(dev, 1, 0xFF);
	ht16k33_setBufferPosition(dev, 2, 0xFF);
	ht16k33_setBufferPosition(dev, 3, 0xFF);
	ht16k33_setBufferPosition(dev, 4, 0xFF);
	ht16k33_flushBufferToDisplay(dev);

	for (uint8_t bri = 0; bri < 16; bri++) {
		ht16k33_setBrightness(dev, bri);
		sleep(1);
	}

	ht16k33_setBlinking(dev, HT16K33_BLINK_2HZ);
	sleep(10);
	ht16k33_setBlinking(dev, HT16K33_BLINK_1HZ);
	sleep(10);
	ht16k33_setBlinking(dev, HT16K33_BLINK_HALFHZ);
	sleep(10);



	ht16k33_turnOff(dev);
	ht16k33_powerOff(dev);
}

