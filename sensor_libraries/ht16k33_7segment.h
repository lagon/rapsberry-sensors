#ifndef __lagon_ht16k33_7seg_h__
#define __lagon_ht16k33_7seg_h__

#include <i2clib.h>

struct ht16k33_7Segment {
	struct ht16k33Device *device;
} ht16k33_7Segment;


struct ht16k33_7Segment *ht16k337Seg_initDevice(int bus_id, uint8_t address);

int ht16k337Seg_setPeriod(struct ht16k33_7Segment *device, uint8_t isShowing, uint8_t pos);
int ht16k337Seg_setColon(struct ht16k33_7Segment *device, uint8_t isShowing);

int ht16k337Seg_printStr(struct ht16k33_7Segment *device, char *string);

int ht16k337Seg_printInteger(struct ht16k33_7Segment *device, int value);
int ht16k337Seg_printDouble(struct ht16k33_7Segment *device, double value);

int ht16k337Seg_powerOn(struct ht16k33_7Segment *device);
int ht16k337Seg_powerOff(struct ht16k33_7Segment *device);

int ht16k337Seg_turnOn(struct ht16k33_7Segment *device);
int ht16k337Seg_turnOff(struct ht16k33_7Segment *device);

int ht16k337Seg_setBrightness(struct ht16k33_7Segment *device, uint8_t brightnessLevel);
int ht16k337Seg_setBlinking(struct ht16k33_7Segment *device, blinkState_t blinkState);

void ht16k337Seg_closeDevice(struct ht16k33_7Segment *device);

void ht16k337Seg_testDisplay();

#endif