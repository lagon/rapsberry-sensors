#ifndef __lagon_bmp183_library_h__
#define __lagon_bmp183_library_h__

#include "spilib.h"

// # d efine __BMP183_DEBUG__

struct bmp183_device {
	int dev;
	int speed;
	short ac1;
	short ac2;
	short ac3;
	unsigned short ac4;
	unsigned short ac5;
	unsigned short ac6;
	short b1;
	short b2;
	short mb;
	short mc;
	short md;
	uint8_t pressureAccuracy;
	long lastRawTemperature;
} bmp183_device;

const extern uint8_t BMP183_ULTRA_LOW_POWER;
const extern uint8_t BMP183_STANDARD;
const extern uint8_t BMP183_HIGH_RESOLUTION;
const extern uint8_t BMP183_ULTRA_HIGH_RESOLUTION;

const extern uint8_t BMP183_MEASURE_TEMPERATURE;
const extern uint8_t BMP183_MEASURE_PRESSURE;

struct bmp183_device *bmp183_init(int busID, int deviceID, int speed, uint8_t pressureAccuracy);
long long bmp183_getMeasurementUSecs(const struct bmp183_device *bmp183, uint8_t measurementType);

int    bmp183_initiateTemeratureMeasurement(const struct bmp183_device *bmp183);
double bmp183_readTemperature(struct bmp183_device *bmp183);

int    bmp183_initiatePressureMeasurement(const struct bmp183_device *bmp183);
double bmp183_readPressure(struct bmp183_device *bmp183);

void bmp183_close(struct bmp183_device *bmp183);

#endif