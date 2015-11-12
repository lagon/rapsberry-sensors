
#ifndef __lagon_h21df_h__
#define __lagon_h21df_h__

#include "i2clib.h"

#include <malloc.h>

struct h21dfDevice {
	int bus_fd;
	uint8_t address;
	double temperature;
	double humidity;
} h21dfDevice;

struct h21dfDevice* h21DF_init(int bus_id);

double h21DF_readTemperature(struct h21dfDevice* dev);

double h21DF_readHumidity(struct h21dfDevice* dev);

void h21DF_close(struct h21dfDevice* dev);

#endif