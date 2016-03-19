#include "tsl2591_luminosity.h"
#include <math.h>

const uint8_t tsl2591_EnableRegister_Addr  = 0x00;
const uint8_t tsl2591_ControlRegister_Addr = 0x01;

const uint8_t tsl2591_PIDRegister_Addr     = 0x11;
const uint8_t tsl2591_IDRegister_Addr      = 0x12;
const uint8_t tsl2591_StatusRegister_Addr  = 0x13;
const uint8_t tsl2591_Channel0_Addr        = 0x14;
const uint8_t tsl2591_Channel1_Addr        = 0x16;

const double TSL2591_LUX_DF = 408.0;
const double TSL2591_LUX_COEFB = 1.64;
const double TSL2591_LUX_COEFC = 0.59;
const double TSL2591_LUX_COEFD = 0.86;

struct tsl2591State *tsl2591_initiate(struct tsl2591State *tsl2591);
void tsl2591_setConfiguration(struct tsl2591State *tsl2591);
uint8_t tsl2591_verifySensorPresent(struct tsl2591State *tsl2591);

struct tsl2591State *tsl2591_initLuminositySensor(uint8_t i2cBusID, uint8_t tsl2591Address) {
	int fd = i2c_initDevice(i2cBusID);
	if (fd < 0) {
		char *errorMsg = (char *) malloc(1024);
		snprintf(errorMsg, 1024, "Error opening I2C connection to tsl2591 luminosity sensor. Message: %s\n", strerror(errno));
		syslog(LOG_ERR, errorMsg);
		free(errorMsg);
		return NULL;
	}
	struct tsl2591State *tsl2591 = (struct tsl2591State*) malloc(sizeof(struct tsl2591State));
	tsl2591->address = tsl2591Address;
	tsl2591->i2cBusDevice = fd;
	tsl2591->integrationTime = IT_200MS;
	tsl2591->sensitivityGain = Gain_Medium;
	tsl2591->autoSensitivity = ManuallySet;


	if (tsl2591_initiate(tsl2591) == NULL) {
		i2c_closeDevice(fd);
		free(tsl2591);
		return NULL;
	}

	tsl2591_setConfiguration(tsl2591);
	return tsl2591;
}

uint8_t constructBusAddress(uint8_t registerAddr) {
	registerAddr = registerAddr & 0x1F;
	return (0x80 | 0x20 | registerAddr);
}

void tsl2591_reset(struct tsl2591State *tsl2591) {
	printf("Issuing reset...\n");
	i2c_write8bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_ControlRegister_Addr), 0xFF);
}

struct tsl2591State *tsl2591_initiate(struct tsl2591State *tsl2591) {
	if (tsl2591_verifySensorPresent(tsl2591) != 0) {
		return NULL;
	}

	tsl2591_reset(tsl2591);
	return tsl2591;
}

uint8_t tsl2591_verifySensorPresent(struct tsl2591State *tsl2591) {
	uint8_t ret = 0xFF;
	if (i2c_read8bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_PIDRegister_Addr), &ret) < 0) {return 1;}
	if ((ret & 0x30) != 0x00) {return 2;}

	if (i2c_read8bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_IDRegister_Addr), &ret) < 0) {return 3;}
	if (ret != 0x50) {return 4;}

	return 0;
}

void tsl2591_setConfiguration(struct tsl2591State *tsl2591) {
	uint8_t gain = 0;
	switch (tsl2591->sensitivityGain) {
		case Gain_Low    : gain = 0x00; break;
		case Gain_Medium : gain = 0x10; break;
		case Gain_High   : gain = 0x20; break;
		case Gain_Max    : gain = 0x30; break;		
	}

	uint8_t intTime = 0;
	switch (tsl2591->integrationTime) {
		case IT_100MS    : intTime = 0x00; break;
		case IT_200MS    : intTime = 0x01; break;
		case IT_300MS    : intTime = 0x02; break;
		case IT_400MS    : intTime = 0x03; break;
		case IT_500MS    : intTime = 0x04; break;
		case IT_600MS    : intTime = 0x05; break;
	}

	uint8_t val = 0x00 | gain | intTime;
	i2c_write8bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_ControlRegister_Addr), val);
}

void tsl2591_enableMeasurement(struct tsl2591State *tsl2591) {
	uint8_t val = 0x03;
	i2c_write8bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_EnableRegister_Addr), val);

	tsl2591_setConfiguration(tsl2591);
}


void tsl2591_disableMeasurement(struct tsl2591State *tsl2591) {
	uint8_t val = 0x00;
	i2c_write8bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_EnableRegister_Addr), val);	
}

void tsl2591_setGain_internal(struct tsl2591State *tsl2591, TSL2591_SensitivityGain_t gain, uint8_t setByAutoSensitity) {
	tsl2591->sensitivityGain = gain;
	tsl2591_setConfiguration(tsl2591);
	if (setByAutoSensitity != 1) {
		tsl2591->autoSensitivity = ManuallySet;
	}
}

void tsl2591_setGain(struct tsl2591State *tsl2591, TSL2591_SensitivityGain_t gain) {
	tsl2591_setGain_internal(tsl2591, gain, 0);
}

void tsl2591_setIntegrationTime_internal(struct tsl2591State *tsl2591, TSL2591_IntegrationTime_t integrationTime, uint8_t setByAutoSensitity) {
	tsl2591->integrationTime = integrationTime;
	tsl2591_setConfiguration(tsl2591);
	if (setByAutoSensitity != 1) {
		tsl2591->autoSensitivity = ManuallySet;
	}
}

void tsl2591_setIntegrationTime(struct tsl2591State *tsl2591, TSL2591_IntegrationTime_t integrationTime) {
	tsl2591_setIntegrationTime_internal(tsl2591, integrationTime, 0);
}

int tsl2591_readRawLuminosityFromChannel(struct tsl2591State *tsl2591, uint8_t channelID, uint16_t *rawLuminosity) {
	if (channelID == 0) {
		if (i2c_read16bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_Channel0_Addr), rawLuminosity) < 0) {return -1;}
	} else if (channelID == 1) {
		if (i2c_read16bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_Channel1_Addr), rawLuminosity) < 0) {return -1;}
	} else {
		return -1;
	}
	return 1;
}

double tsl2591_readLuminosity(struct tsl2591State *tsl2591) {
	uint16_t ch0, ch1;

	if (tsl2591_readRawLuminosityFromChannel(tsl2591, 0, &ch0) < 0) {return NAN;}
	if (tsl2591_readRawLuminosityFromChannel(tsl2591, 1, &ch1) < 0) {return NAN;}

	if ((ch0 == 0xFFFF) | (ch1 == 0xFFFF)) {
		return INFINITY;
	}
	if ((ch0 < 0x0010) | (ch1 < 0x0010)) {
		return -INFINITY;
	}

	double gain, itime;
	switch (tsl2591->sensitivityGain) {
		case Gain_Low    : gain = 1.0; break;
		case Gain_Medium : gain = 25.0; break;
		case Gain_High   : gain = 428.0; break;
		case Gain_Max    : gain = 9876.0; break;		
		default: gain = 0.0;
	}

	switch (tsl2591->integrationTime) {
		case IT_100MS    : itime = 100.0; break;
		case IT_200MS    : itime = 200.0; break;
		case IT_300MS    : itime = 300.0; break;
		case IT_400MS    : itime = 400.0; break;
		case IT_500MS    : itime = 500.0; break;
		case IT_600MS    : itime = 600.0; break;
		default: itime = 0.0;
	}

	double cpl = (gain * itime) / TSL2591_LUX_DF;
	printf("                     %0.4g * %0.4g / %0.4g = %0.4g\n", gain, itime, TSL2591_LUX_DF, cpl);
// const double TSL2591_LUX_DF = 408.0;
// const double TSL2591_LUX_COEFB = 1.64;
// const double TSL2591_LUX_COEFC = 0.59;
// const double TSL2591_LUX_COEFD = 0.86;

// 	double lux1 = ((double) ch0 - (TSL2591_LUX_COEFB * (double) ch1)) / cpl;
//   	double lux2 = ((TSL2591_LUX_COEFC * (double) ch0) - (TSL2591_LUX_COEFD * (double)ch1 ) ) / cpl;
//   	double lux = lux1 > lux2 ? lux1 : lux2;
	double lux = ((double) ch0 - 1.7 * (double) ch1) / cpl;
  	printf("                     Lux read: %0.4g = %d - 1.7 * %d / %0.4g\n", lux, ch0, ch1, cpl);
  	return lux;
}

void tsl2591_SetGainAndTimeAccordingToAutoSensitivity(struct tsl2591State *tsl2591) {
	switch (tsl2591->autoSensitivity) {
		case Low_100MS:
		case Low_200MS:
		case Low_300MS:
		case Low_400MS:
		case Low_500MS:
		case Low_600MS: tsl2591_setGain_internal(tsl2591, Gain_Low, 1); break;

		case Med_100MS:
		case Med_200MS:
		case Med_300MS:
		case Med_400MS:
		case Med_500MS:
		case Med_600MS: tsl2591_setGain_internal(tsl2591, Gain_Medium, 1); break;

		case Hig_100MS:
		case Hig_200MS:
		case Hig_300MS:
		case Hig_400MS:
		case Hig_500MS:
		case Hig_600MS: tsl2591_setGain_internal(tsl2591, Gain_High, 1); break;

		case Max_100MS:
		case Max_200MS:
		case Max_300MS:
		case Max_400MS:
		case Max_500MS:
		case Max_600MS: tsl2591_setGain_internal(tsl2591, Gain_Max, 1); break;
		default: break;
	}

	switch (tsl2591->autoSensitivity) {
		case Low_100MS:
		case Med_100MS:
		case Hig_100MS:
		case Max_100MS: tsl2591_setIntegrationTime_internal(tsl2591, IT_100MS, 1); break;

		case Low_200MS:
		case Med_200MS:
		case Hig_200MS:
		case Max_200MS: tsl2591_setIntegrationTime_internal(tsl2591, IT_200MS, 1); break;

		case Low_300MS:
		case Med_300MS:
		case Hig_300MS:
		case Max_300MS: tsl2591_setIntegrationTime_internal(tsl2591, IT_300MS, 1); break;

		case Low_400MS:
		case Med_400MS:
		case Hig_400MS:
		case Max_400MS: tsl2591_setIntegrationTime_internal(tsl2591, IT_400MS, 1); break;

		case Low_500MS:
		case Med_500MS:
		case Hig_500MS:
		case Max_500MS: tsl2591_setIntegrationTime_internal(tsl2591, IT_500MS, 1); break;

		case Low_600MS:
		case Med_600MS:
		case Hig_600MS:
		case Max_600MS: tsl2591_setIntegrationTime_internal(tsl2591, IT_600MS, 1); break;
	
		default: break;
	}
}

void tsl2591_IncreaseSensitivity(struct tsl2591State *tsl2591) {
	switch (tsl2591->autoSensitivity) {
		case Low_100MS: tsl2591->autoSensitivity = Low_200MS; break;
		case Low_200MS: tsl2591->autoSensitivity = Low_300MS; break;
		case Low_300MS: tsl2591->autoSensitivity = Low_400MS; break;
		case Low_400MS: tsl2591->autoSensitivity = Low_500MS; break;
		case Low_500MS: tsl2591->autoSensitivity = Low_600MS; break;
		case Low_600MS: tsl2591->autoSensitivity = Med_100MS; break;

		case Med_100MS: tsl2591->autoSensitivity = Med_200MS; break;
		case Med_200MS: tsl2591->autoSensitivity = Med_300MS; break;
		case Med_300MS: tsl2591->autoSensitivity = Med_400MS; break;
		case Med_400MS: tsl2591->autoSensitivity = Med_500MS; break;
		case Med_500MS: tsl2591->autoSensitivity = Med_600MS; break;
		case Med_600MS: tsl2591->autoSensitivity = Hig_100MS; break;

		case Hig_100MS: tsl2591->autoSensitivity = Hig_200MS; break;
		case Hig_200MS: tsl2591->autoSensitivity = Hig_300MS; break;
		case Hig_300MS: tsl2591->autoSensitivity = Hig_400MS; break;
		case Hig_400MS: tsl2591->autoSensitivity = Hig_500MS; break;
		case Hig_500MS: tsl2591->autoSensitivity = Hig_600MS; break;
		case Hig_600MS: tsl2591->autoSensitivity = Max_100MS; break;
		
		case Max_100MS: tsl2591->autoSensitivity = Max_200MS; break;
		case Max_200MS: tsl2591->autoSensitivity = Max_300MS; break;
		case Max_300MS: tsl2591->autoSensitivity = Max_400MS; break;
		case Max_400MS: tsl2591->autoSensitivity = Max_500MS; break;
		case Max_500MS: tsl2591->autoSensitivity = Max_600MS; break;
		case Max_600MS: tsl2591->autoSensitivity = Max_600MS; break;

		default: break;
	}
}

void tsl2591_DecreaseSensitivity(struct tsl2591State *tsl2591) {
	switch (tsl2591->autoSensitivity) {
		case Low_100MS: tsl2591->autoSensitivity = Low_100MS; break;
		case Low_200MS: tsl2591->autoSensitivity = Low_100MS; break;
		case Low_300MS: tsl2591->autoSensitivity = Low_200MS; break;
		case Low_400MS: tsl2591->autoSensitivity = Low_300MS; break;
		case Low_500MS: tsl2591->autoSensitivity = Low_400MS; break;
		case Low_600MS: tsl2591->autoSensitivity = Low_500MS; break;

		case Med_100MS: tsl2591->autoSensitivity = Low_600MS; break;
		case Med_200MS: tsl2591->autoSensitivity = Med_100MS; break;
		case Med_300MS: tsl2591->autoSensitivity = Med_200MS; break;
		case Med_400MS: tsl2591->autoSensitivity = Med_300MS; break;
		case Med_500MS: tsl2591->autoSensitivity = Med_400MS; break;
		case Med_600MS: tsl2591->autoSensitivity = Med_500MS; break;

		case Hig_100MS: tsl2591->autoSensitivity = Med_600MS; break;
		case Hig_200MS: tsl2591->autoSensitivity = Hig_100MS; break;
		case Hig_300MS: tsl2591->autoSensitivity = Hig_200MS; break;
		case Hig_400MS: tsl2591->autoSensitivity = Hig_300MS; break;
		case Hig_500MS: tsl2591->autoSensitivity = Hig_400MS; break;
		case Hig_600MS: tsl2591->autoSensitivity = Hig_500MS; break;
		
		case Max_100MS: tsl2591->autoSensitivity = Hig_600MS; break;
		case Max_200MS: tsl2591->autoSensitivity = Max_100MS; break;
		case Max_300MS: tsl2591->autoSensitivity = Max_200MS; break;
		case Max_400MS: tsl2591->autoSensitivity = Max_300MS; break;
		case Max_500MS: tsl2591->autoSensitivity = Max_400MS; break;
		case Max_600MS: tsl2591->autoSensitivity = Max_500MS; break;

		default: break;
	}
}

void tsl2591_PrintSensitivity(struct tsl2591State *tsl2591) {
	switch (tsl2591->autoSensitivity) {
		case Low_100MS: printf("Low_100MS\n"); break;
		case Low_200MS: printf("Low_200MS\n"); break;
		case Low_300MS: printf("Low_300MS\n"); break;
		case Low_400MS: printf("Low_400MS\n"); break;
		case Low_500MS: printf("Low_500MS\n"); break;
		case Low_600MS: printf("Low_600MS\n"); break;

		case Med_100MS: printf("Med_100MS\n"); break;
		case Med_200MS: printf("Med_200MS\n"); break;
		case Med_300MS: printf("Med_300MS\n"); break;
		case Med_400MS: printf("Med_400MS\n"); break;
		case Med_500MS: printf("Med_500MS\n"); break;
		case Med_600MS: printf("Med_600MS\n"); break;

		case Hig_100MS: printf("Hig_100MS\n"); break;
		case Hig_200MS: printf("Hig_200MS\n"); break;
		case Hig_300MS: printf("Hig_300MS\n"); break;
		case Hig_400MS: printf("Hig_400MS\n"); break;
		case Hig_500MS: printf("Hig_500MS\n"); break;
		case Hig_600MS: printf("Hig_600MS\n"); break;

		case Max_100MS: printf("Max_100MS\n"); break;
		case Max_200MS: printf("Max_200MS\n"); break;
		case Max_300MS: printf("Max_300MS\n"); break;
		case Max_400MS: printf("Max_400MS\n"); break;
		case Max_500MS: printf("Max_500MS\n"); break;
		case Max_600MS: printf("Max_600MS\n"); break;

		default: printf("ManuallySet"); break;
	}
}



uint8_t tsl2591_isReadingValid(struct tsl2591State *tsl2591) {
	uint8_t val;
	if (i2c_read8bits(tsl2591->i2cBusDevice, tsl2591->address, constructBusAddress(tsl2591_StatusRegister_Addr), &val) < 0) {return 0;}
//	printf("0x%0X\n", val);
	return (val & 0x01);
}

void tsl2591_waitUntilReadingValid(struct tsl2591State *tsl2591) {
	while(tsl2591_isReadingValid(tsl2591) == 0) {usleep(100000); printf("."); fflush(NULL);}
	//usleep(700000);
}

double tsl2591_readLuminosityWithAutoSensitivity(struct tsl2591State *tsl2591) {
	if (tsl2591->autoSensitivity == ManuallySet) {
		tsl2591->autoSensitivity = Med_100MS;
		tsl2591_SetGainAndTimeAccordingToAutoSensitivity(tsl2591);
		printf("Sensitivity set to: "); tsl2591_PrintSensitivity(tsl2591);
	}

	tsl2591_waitUntilReadingValid(tsl2591);
	double lux = tsl2591_readLuminosity(tsl2591);

	while(isinf(lux)) {
		if (isnan(lux)) {
			return NAN;
		}

		TSL2591_AutoSensitivityOrder_t lastSensitivity = tsl2591->autoSensitivity;

		if (lux < 0) {
			tsl2591_IncreaseSensitivity(tsl2591);
		} else {
			tsl2591_DecreaseSensitivity(tsl2591);
		}

		if (lastSensitivity == tsl2591->autoSensitivity) {
			return lux;
		}
		tsl2591_SetGainAndTimeAccordingToAutoSensitivity(tsl2591);	
		printf("Sensitivity set to: "); tsl2591_PrintSensitivity(tsl2591);
		
		tsl2591_waitUntilReadingValid(tsl2591);
		lux = tsl2591_readLuminosity(tsl2591);		
	}

	return lux;
}

void tsl2591_destroyTemperatureSensorState(struct tsl2591State *tsl2591) {
	tsl2591_disableMeasurement(tsl2591);
	i2c_closeDevice(tsl2591->i2cBusDevice);
	free(tsl2591);
}

void tsl2591_testMeasurement() {
	printf("Initiating...\n");
	struct tsl2591State *state = tsl2591_initLuminositySensor(1, 0x29);

	printf("Enabling...\n");
	tsl2591_enableMeasurement(state);
	while(1) {
		sleep(1);
		printf("Luminosity is : %0.4g lux\n", tsl2591_readLuminosityWithAutoSensitivity(state));
	}
}