#ifndef __lagon_ht16k33_h__
#define __lagon_ht16k33_h__

#include <i2clib.h>

#define _ht16k33_bufferSize 5

typedef enum {HT16K33_NO_BLINK = 0, HT16K33_BLINK_2HZ  = 1, HT16K33_BLINK_1HZ = 2, HT16K33_BLINK_HALFHZ = 3}  blinkState_t;

struct ht16k33Device {
	int bus_fd;
	uint8_t address;
	
	blinkState_t blinkState;
	uint8_t displayShowing;
	uint8_t brightness;

	uint8_t buffer[(_ht16k33_bufferSize * 2) + 1];
} ht16k33Device;

struct ht16k33Device *ht16k33_initDevice(int bus_id, uint8_t address);

int ht16k33_powerOn(struct ht16k33Device *device);
int ht16k33_powerOff(struct ht16k33Device *device);

int ht16k33_turnOn(struct ht16k33Device *device);
int ht16k33_turnOff(struct ht16k33Device *device);

int ht16k33_setBrightness(struct ht16k33Device *device, uint8_t brightnessLevel);
int ht16k33_setBlinking(struct ht16k33Device *device, blinkState_t blinkState);

int ht16k33_setBufferPosition(struct ht16k33Device *device, uint8_t position, uint16_t value);
int ht16k33_getBufferPosition(struct ht16k33Device *device, uint8_t position, uint16_t *value);
int ht16k33_flushBufferToDisplay(struct ht16k33Device *device);

int ht16k33_closeDevice(struct ht16k33Device *device);

void ht16k33_testDisplay();

#endif