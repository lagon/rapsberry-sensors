#ifndef __lagon_mpr121_library_h__
#define __lagon_mpr121_library_h__

#include <inttypes.h>

struct mpr121_device {
	int bus_fd;
	uint8_t address;
	long long measurementRefreshIntervalUsec;
	long long lastRefresh;

	uint16_t touchStatus;
	uint8_t dataValid;

	uint8_t isRunningMode;
	
	uint8_t maxElectrodeToTrack;
	uint8_t proximitySensorSize;
};


int mpr121_isElectrodeTouched(struct mpr121_device *dev, uint8_t electrodeID);
uint16_t mpr121_getElectrodeFilteredValues(struct mpr121_device *dev, uint8_t electrodeID);
uint16_t mpr121_getElectrodeBaseLineValue(struct mpr121_device *dev, uint8_t electrodeID);
int mpr121_resetAndSetup(struct mpr121_device *dev);
int mpr121_isAutoConfigurastionDone(struct mpr121_device *dev);
int mpr121_putToStopMode(struct mpr121_device *dev);
int mpr121_putToRunningMode(struct mpr121_device *dev);
struct mpr121_device *mpr121_initializeWithAllElectrodesEnabled(int bus_id, uint8_t address);
void mpr121_finishAndClose(struct mpr121_device *dev);

#endif