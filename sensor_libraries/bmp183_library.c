#include "bmp183_library.h"
#include <math.h>

const uint8_t BMP183_AC1_ADDR_RD = 0xAA | 0x80;
const uint8_t BMP183_AC2_ADDR_RD = 0xAC | 0x80;
const uint8_t BMP183_AC3_ADDR_RD = 0xAE | 0x80;
const uint8_t BMP183_AC4_ADDR_RD = 0xB0 | 0x80;
const uint8_t BMP183_AC5_ADDR_RD = 0xB2 | 0x80;
const uint8_t BMP183_AC6_ADDR_RD = 0xB4 | 0x80;

const uint8_t BMP183_B1_ADDR_RD  = 0xB6 | 0x80;
const uint8_t BMP183_B2_ADDR_RD  = 0xB8 | 0x80;

const uint8_t BMP183_MB_ADDR_RD  = 0xBA | 0x80;
const uint8_t BMP183_MC_ADDR_RD  = 0xBC | 0x80;
const uint8_t BMP183_MD_ADDR_RD  = 0xBE | 0x80;

const uint8_t BMP183_COMMAND_ADDR_WR        = 0xF4 & 0x7F;
const uint8_t BMP183_MEASURED_VALUE_ADDR_RD = 0xF6 | 0x80;

const uint8_t BMP183_ULTRA_LOW_POWER       = 0x34;
const uint8_t BMP183_STANDARD              = 0x74;
const uint8_t BMP183_HIGH_RESOLUTION       = 0xB4;
const uint8_t BMP183_ULTRA_HIGH_RESOLUTION = 0xF4;

const uint8_t BMP183_MEASURE_TEMPERATURE   = 0x01;
const uint8_t BMP183_MEASURE_PRESSURE      = 0x02;


struct bmp183_device *bmp183_init(int busID, int deviceID, int speed, uint8_t pressureAccuracy) {
	if ((pressureAccuracy != BMP183_ULTRA_LOW_POWER) && 
		(pressureAccuracy != BMP183_STANDARD) && 
		(pressureAccuracy != BMP183_HIGH_RESOLUTION) && 
		(pressureAccuracy != BMP183_ULTRA_HIGH_RESOLUTION)) {
        char *msg = (char *)malloc(sizeof(char) * 1024);
        snprintf(msg, 1024, "Pressure accuracy %d is not valid value. (Must be one of %d, %d, %d, %d).", pressureAccuracy, BMP183_ULTRA_LOW_POWER, BMP183_STANDARD, BMP183_HIGH_RESOLUTION, BMP183_ULTRA_HIGH_RESOLUTION);
        syslog(LOG_ERR, msg);
        free(msg);
		return NULL;
	}

	int fd = spi_initDevice(busID, deviceID);
	if (fd < 0) {
		return NULL;
	}

	struct bmp183_device *bmp183 = (struct bmp183_device *) malloc(sizeof(struct bmp183_device));

	bmp183->dev = fd;
	bmp183->speed = speed;
	uint16_t tmp;

#ifdef __BMP183_DEBUG__
	bmp183->ac1 = 408;
	bmp183->ac2 = -72;
	bmp183->ac3 = -14383;
	bmp183->ac4 = 32741;
	bmp183->ac5 = 32757;
	bmp183->ac6 = 23153;

	bmp183->b1 = 6190;
	bmp183->b2 = 4;

	bmp183->mb = -32767;
	bmp183->mc = -8711;
	bmp183->md = 2868;
#else
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_AC1_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->ac1 = (short) tmp;
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_AC2_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->ac2 = (short) tmp;
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_AC3_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->ac3 = (short) tmp;
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_AC4_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->ac4 = (unsigned short) tmp;
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_AC5_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->ac5 = (unsigned short) tmp;
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_AC6_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->ac6 = (unsigned short) tmp;

	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_B1_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->b1 = (short) tmp;
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_B2_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->b2 = (short) tmp;

	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_MB_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->mb = (short) tmp;
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_MC_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;}
	bmp183->mc = (short) tmp;
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_MD_ADDR_RD, 1, &tmp, bmp183->speed, 8, 1) < 0) {free(bmp183); return NULL;} 
	bmp183->md = (short) tmp;
#endif

	bmp183->pressureAccuracy = pressureAccuracy;
	bmp183->lastRawTemperature = 0;

#ifdef __BMP183_DEBUG__
	printf("AC1 = %d\n", bmp183->ac1);
	printf("AC2 = %d\n", bmp183->ac2);
	printf("AC3 = %d\n", bmp183->ac3);
	printf("AC4 = %d\n", bmp183->ac4);
	printf("AC5 = %d\n", bmp183->ac5);
	printf("AC6 = %d\n", bmp183->ac6);

	printf("B1 = %d\n", bmp183->b1);
	printf("B2 = %d\n", bmp183->b2);

	printf("MB = %d\n", bmp183->mb);
	printf("MC = %d\n", bmp183->mc);
	printf("MD = %d\n", bmp183->md);
#endif

	return bmp183;
};

long long bmp183_getMeasurementUSecs(const struct bmp183_device *bmp183, uint8_t measurementType) {
	if (measurementType == BMP183_MEASURE_TEMPERATURE) {
		return 4500l;
	}
	if (measurementType == BMP183_MEASURE_PRESSURE) {
		if (bmp183->pressureAccuracy == BMP183_ULTRA_LOW_POWER) {return 4500l;}
		if (bmp183->pressureAccuracy == BMP183_STANDARD)        {return 7500l;}
		if (bmp183->pressureAccuracy == BMP183_HIGH_RESOLUTION) {return 13500l;}
		if (bmp183->pressureAccuracy == BMP183_ULTRA_HIGH_RESOLUTION) {return 25500l;}
		
		return -1;
	}
	return -1;
}


int bmp183_initiateTemeratureMeasurement(const struct bmp183_device *bmp183) {
	return spi_write8bToAnyAddress(bmp183->dev, &BMP183_COMMAND_ADDR_WR, 1, 0x2E, bmp183->speed, 8, 0);
}

double bmp183_readTemperature(struct bmp183_device *bmp183) {
	uint16_t rawTemp;

#ifdef __BMP183_DEBUG__
	rawTemp = 27898;
#else
	if (spi_read16bFromAnyAddress(bmp183->dev, &BMP183_MEASURED_VALUE_ADDR_RD, 1, &rawTemp, bmp183->speed, 8, 1) < 0) {
		return NAN;
	};

#endif
	bmp183->lastRawTemperature = rawTemp;
	// printf("rawTemp = %d\n", rawTemp);	

	long x1 = (((long)rawTemp - (long)bmp183->ac6) * (long)bmp183->ac5) >> 15;
	// printf("x1 = %ld\n", x1);
	long x2 = ((long)bmp183->mc << 11) / (x1 + (long)bmp183->md);
	// printf("x2 = %ld\n", x2);
	long b5 = x1 + x2;
	// printf("b5 = %ld\n", b5);
	long t = (b5 + 8) >> 4;
	// printf("t = %ld\n", t);

	return ((double) t) / 10.0;
}


int bmp183_initiatePressureMeasurement(const struct bmp183_device *bmp183) {
	return spi_write8bToAnyAddress(bmp183->dev, &BMP183_COMMAND_ADDR_WR, 1, bmp183->pressureAccuracy, bmp183->speed, 8, 0);
}

double bmp183_readPressure(struct bmp183_device *bmp183) {
	uint8_t *p_rawPress = (uint8_t *) malloc(sizeof(uint8_t) * 3);
	uint32_t rawPress = 0;

#ifdef __BMP183_DEBUG__
	rawPress = 23843;
	bmp183->lastRawTemperature = 27898;
	uint8_t oss = 0;
#else
	uint8_t oss = (bmp183->pressureAccuracy & 0xC0) >> 6;

	if (spi_readBytesFromAnyAddress(bmp183->dev, &BMP183_MEASURED_VALUE_ADDR_RD, 1, p_rawPress, 3, bmp183->speed, 8, 1) < 0) {
		return NAN;
	};
	
	rawPress = (p_rawPress[0] << 16) + (p_rawPress[1] << 8) + p_rawPress[2];
	rawPress = rawPress >> (8 - oss);

	// printf("rawPress = %ld  (%X %X %X)\n", rawPress, p_rawPress[0], p_rawPress[1], p_rawPress[2]);	
	// printf("rawTemp = %ld\n", bmp183->lastRawTemperature);	
#endif
	// printf("oss = %d\n", oss);

	long x1 = (((long)bmp183->lastRawTemperature - (long)bmp183->ac6) * (long)bmp183->ac5) >> 15;
	// printf("x1 = %ld\n", x1);
	long x2 = ((long)bmp183->mc << 11) / (x1 + (long)bmp183->md);
	// printf("x2 = %ld\n", x2);
	long b5 = x1 + x2;
	// printf("b5 = %ld\n", b5);


	long b6 = b5 - 4000;
	// printf("b6 = %ld\n", b6);
	x1 = ((long)bmp183->b2 * ((b6 * b6) >> 12)) >> 11;
	// printf("x1 = %ld\n", x1);
	x2 = ((long)bmp183->ac2 * b6) >> 11;
	// printf("x2 = %ld\n", x2);
	long x3 = x1 + x2;
	// printf("x3 = %ld\n", x3);
	long b3 = ((((long)bmp183->ac1 * 4l + x3) << oss) + 2) / 4;
	// printf("b3 = %ld\n", b3);
	x1 = ((long)bmp183->ac3 * b6) >> 13;
	// printf("x1 = %ld\n", x1);
	x2 = ((long)bmp183->b1 * ((b6 * b6) >> 12)) >> 16;
	// printf("x2 = %ld\n", x2);
	x3 = ((x1 + x2) + 2) / 4;
	// printf("x3 = %ld\n", x3);
	unsigned long b4 = (long)bmp183->ac4 * (unsigned long) (x3 + 32768) >> 15;
	// printf("b4 = %ld\n", b4);
	long b7 = ((long)(((unsigned long) rawPress) - (long)b3)) * (50000 >> oss);
	// printf("b7 = %ld\n", b7);
	long p;
	if (b7 < 0x80000000) {
		p = (b7 * 2) / b4;
	} else {
		p = (b7 / b4) * 2;
	}
	// printf("p = %ld\n", p);
	x1 = (p / 256.0) * (p / 256.0);
	// printf("x1 = %ld\n", x1);
	x1 = (x1 * 3038) >> 16;
	// printf("x1 = %ld\n", x1);
	x2 = (-7357 * p) >> 16;
	// printf("x2 = %ld\n", x2);
	p = p + ((x1 + x2 + 3791l) >> 4);
	// printf("p = %ld\n", p);

	free(p_rawPress);
	return (double) p;
}

void bmp183_close(struct bmp183_device *bmp183) {
	spi_closeDevice(bmp183->dev);
	free(bmp183);
}
