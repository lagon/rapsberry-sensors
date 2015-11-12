#include "h21df_library.h"

#include <malloc.h>
#include <syslog.h>
#include <math.h>
#include <unistd.h>

static const int MAX_TEMP_CONVERSION    = 50;   // milliseconds
static const int MAX_HUMI_CONVERSION    = 16;   // ms
static const int MAX_RESET_DELAY        = 15;   // ms

static uint8_t HTU21DF_READTEMP_NH      = 0xF3; // NH = no hold
static uint8_t HTU21DF_READHUMI_NH      = 0xF5;
//static uint8_t HTU21DF_WRITEREG         = 0xE6;
//static uint8_t HTU21DF_READREG          = 0xE7;
static uint8_t HTU21DF_RESET            = 0xFE;

static const double HTU21DF_TEMPERATURE_COMPENSATION_COEFICIENT = -0.15;


int h21DF_checkCRC(unsigned char *data, int len) {
	if (len != 3) {
		syslog(LOG_ERR, "CRC Failed - invalid data length received.");
		return -1;
	}

    const uint32_t start_polynomial = 0x98800000;
    uint32_t dataandcrc = (data[0] << 24) | (data[1] << 16) | (data[2] << 8);
    for (int i = 0; i < 24; i++) {
        if (dataandcrc & 0x80000000UL)
            dataandcrc ^= start_polynomial;
        dataandcrc <<= 1;
    }
    return dataandcrc;
}


struct h21dfDevice* h21DF_init(int bus_id) {
	int fd = i2c_initDevice(bus_id);
	if (fd < 0) {
		return NULL;
	}
	struct h21dfDevice* dev = (struct h21dfDevice *) malloc(sizeof(struct h21dfDevice));

	dev->bus_fd = fd;
	dev->address = 0x40;
	dev->temperature = NAN;
	dev->humidity = NAN;

	unsigned char data[10];
    data[0] = HTU21DF_RESET;

    if (i2c_writeToDevice(dev->bus_fd, dev->address, data, 1) != 1) {
        syslog(LOG_ERR, "Error sending RESET");
        return NULL;
    }
    msleep(MAX_RESET_DELAY);

    return dev;
}



double h21DF_readTemperature(struct h21dfDevice* dev) {
	unsigned char data[10];
    data[0] = HTU21DF_READTEMP_NH;

	int w_bytes = i2c_writeToDevice(dev->bus_fd, dev->address, data, 1);
	if (w_bytes < 0) {
		syslog(LOG_ERR, "Unable to write temperature command to H21DF.");
		dev->temperature = NAN;
		return NAN;
	}

	msleep(MAX_TEMP_CONVERSION);

	int r_bytes = i2c_readFromDevice(dev->bus_fd, dev->address, data, 3);
	if (r_bytes != 3) {
		syslog(LOG_ERR, "Unable to read temperature from H21DF.");
		dev->temperature = NAN;
		return NAN;
	}

	if (h21DF_checkCRC(data, 3) != 0) {
		syslog(LOG_ERR, "CRC temperature check failed - from H21DF.");
		dev->temperature = NAN;
		return NAN;
	}

	uint16_t rawTemp = ((data[0] << 8) | data[1]) & 0xFFFC;
	dev->temperature = -46.85 + (175.72 * ((double)rawTemp / (65536.0)));
	return dev->temperature;
}

double h21DF_readHumidity(struct h21dfDevice* dev) {
	unsigned char data[10];
    data[0] = HTU21DF_READHUMI_NH;

	int w_bytes = i2c_writeToDevice(dev->bus_fd, dev->address, data, 1);
	if (w_bytes < 0) {
		syslog(LOG_ERR, "Unable to write humidity command to H21DF.");
		dev->humidity = NAN;
		return NAN;
	}

	msleep(MAX_HUMI_CONVERSION);

	int r_bytes = i2c_readFromDevice(dev->bus_fd, dev->address, data, 3);
	if (r_bytes != 3) {
		syslog(LOG_ERR, "Unable to read humidity from H21DF.");
		dev->humidity = NAN;
		return NAN;
	}

	if (h21DF_checkCRC(data, 3) != 0) {
		syslog(LOG_ERR, "CRC humidity check failed - from H21DF.");
		dev->humidity = NAN;
		return NAN;
	}

	uint16_t rawHumidity = ((data[0] << 8) | data[1]) & 0xFFFC;
	dev->humidity = -6.0 + ((125.0 / 65536.0) * (double)rawHumidity);
	if (dev->temperature != NAN) {
		dev->humidity = dev->humidity + (25 - dev->temperature) * HTU21DF_TEMPERATURE_COMPENSATION_COEFICIENT;
	}

	return dev->humidity;
}

void h21DF_close(struct h21dfDevice* dev) {
	i2c_closeDevice(dev->bus_fd);
	free(dev);
}

