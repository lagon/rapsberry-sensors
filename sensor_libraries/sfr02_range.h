#ifndef __lagon_sfr02_distance_h__
#define __lagon_sfr02_distance_h__

#include <inttypes.h>
#include <i2clib.h>


typedef enum {SFR02_INCH, SFR02_CM, SFR02_USEC}  sfr02_units_reading_t;

struct sfr02Device {
	int bus_fd;
	uint8_t address;

	sfr02_units_reading_t units;
};

struct sfr02Device *sfr02_initDevice(int bus_id, uint8_t busAddr);

int sfr02_setMeasurementUnits(struct sfr02Device *device, sfr02_units_reading_t units);
sfr02_units_reading_t sfr02_getMeasurementUnits(struct sfr02Device *device);

int sfr02_initiateReading(struct sfr02Device *device);
int sfr02_LastReadingValue(struct sfr02Device *device, uint16_t *lastValue);

int sfr02_closeDevice(struct sfr02Device *device);

int sfr02_testRanging();

#endif