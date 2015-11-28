#include "led_driver_library.h"

#include <syslog.h>

void printHexBuffer(void *buffer, int length);

void split16bInt(uint16_t value, uint8_t output [2]) {
	output[0] = (value >> 8);
	output[1] = (value & 0x00FF);
	return;
}

uint8_t *constructPacketRGB(struct ledPWMSettings settings, struct globalBrightness brightness, struct rgbData rgb[4], uint8_t *out_data) {
	uint8_t * pckt = out_data;
	uint32_t constructionPad = 0;
	uint8_t *constPadPtr = (uint8_t *) &constructionPad;
	constructionPad = 0x25 << 26;
	constructionPad = constructionPad | ((settings.outtmg == 0 ? 0 : 1) << 25);
	constructionPad = constructionPad | ((settings.extgck == 0 ? 0 : 1) << 24);
	constructionPad = constructionPad | ((settings.tmgrst == 0 ? 0 : 1) << 23);
	constructionPad = constructionPad | ((settings.dsprpt == 0 ? 0 : 1) << 22);
	constructionPad = constructionPad | ((settings.blank  == 0 ? 0 : 1) << 21);
	constructionPad = constructionPad | ((brightness.rBrightness & 0x7F) << 14);
	constructionPad = constructionPad | ((brightness.gBrightness & 0x7F) <<  7);
	constructionPad = constructionPad | ((brightness.bBrightness & 0x7F) <<  0);

	for (int i = 0; i < 4; i++) {
		pckt[i] = constPadPtr[3-i];
	}

	uint8_t pad [2];
	pckt = pckt + 4;
	for (int i = 0; i < 4; i++) {
		split16bInt(rgb[3-i].b, pad);
		memcpy(pckt + (i * 6), pad, 2);
		split16bInt(rgb[3-i].g, pad);
		memcpy(pckt + (i * 6 + 2), pad, 2);
		split16bInt(rgb[3-i].r, pad);
		memcpy(pckt + (i * 6 + 4), pad, 2);
	}

	return out_data;
}


struct allLedControlStruct *initiateLEDControls(uint8_t numRGBLeds) {
	struct allLedControlStruct *allLed = (struct allLedControlStruct *) malloc(sizeof(struct allLedControlStruct));
	if (numRGBLeds % 4 != 0) {
		numRGBLeds = numRGBLeds + (4 - numRGBLeds % 4);
	}
	allLed->numLeds = numRGBLeds;
	allLed->settings.outtmg = 0;
	allLed->settings.extgck = 0;
	allLed->settings.tmgrst = 0;
	allLed->settings.dsprpt = 0;
	allLed->settings.blank = 0;

	allLed->brightness.rBrightness = 0;
	allLed->brightness.gBrightness = 0;
	allLed->brightness.bBrightness = 0;

	allLed->individualLeds = (struct rgbData*) malloc((numRGBLeds) * sizeof(rgbData));

	memset(allLed->individualLeds, 0, (numRGBLeds) * sizeof(rgbData));
	return(allLed);
}

void setLedSettings(struct allLedControlStruct *ledStructure, struct ledPWMSettings *settings) {
	ledStructure->settings.blank  = settings->blank;
	ledStructure->settings.dsprpt = settings->dsprpt;
	ledStructure->settings.tmgrst = settings->tmgrst;
	ledStructure->settings.extgck = settings->extgck;
	ledStructure->settings.outtmg = settings->outtmg;
}

void setGlobalBrightness(struct allLedControlStruct *ledStructure, uint8_t brightness) {
	ledStructure->brightness.rBrightness = brightness & 0x7F;
	ledStructure->brightness.gBrightness = brightness & 0x7F;
	ledStructure->brightness.bBrightness = brightness & 0x7F;
}

void setGlobalBrightnessRGB(struct allLedControlStruct *ledStructure, const struct rgbData *rgb) {
	ledStructure->brightness.rBrightness = rgb->r & 0x7F;
	ledStructure->brightness.gBrightness = rgb->g & 0x7F;
	ledStructure->brightness.bBrightness = rgb->b & 0x7F;
}

void setOneLedRGB(struct allLedControlStruct *ledStructure, uint8_t ledIndex, const struct rgbData *rgb) {
	if (ledIndex >= ledStructure->numLeds) {
		syslog(LOG_ERR, "Invalid LED index passed");
		return;
	}

	ledStructure->individualLeds[ledIndex].r = rgb->r;
	ledStructure->individualLeds[ledIndex].g = rgb->g;
	ledStructure->individualLeds[ledIndex].b = rgb->b;
}

void setThreeGrayscaleLed(struct allLedControlStruct *ledStructure, uint8_t ledIndex, uint16_t intensity1, uint16_t intensity2, uint16_t intensity3) {
	if (ledIndex >= ledStructure->numLeds) {
		syslog(LOG_ERR, "Invalid LED index passed");
		return;
	}
	ledStructure->individualLeds[ledIndex].r = intensity1;
	ledStructure->individualLeds[ledIndex].g = intensity2;
	ledStructure->individualLeds[ledIndex].b = intensity3;
}

void setOneGrayscaleLed(struct allLedControlStruct *ledStructure, uint8_t gsLedIndex, uint16_t intensity) {
	uint8_t ledIndex = gsLedIndex / 3;
	if (ledIndex >= ledStructure->numLeds) {
		syslog(LOG_ERR, "Invalid LED index passed");
		return;
	}
	switch (gsLedIndex % 3) {
		case 0:
			ledStructure->individualLeds[ledIndex].r = intensity;
			break;
		case 1:
			ledStructure->individualLeds[ledIndex].g = intensity;
			break;
		case 2:
			ledStructure->individualLeds[ledIndex].b = intensity;
			break;
	}
}


void sendOutLedDataDefaults(struct allLedControlStruct *ledStructure, const int spi_device) {
	sendOutLedData(ledStructure, spi_device, 50000, 8, 0);
}

void sendOutLedData(struct allLedControlStruct *ledStructure, const int spi_device, const int speed, const unsigned int bits_per_word, const unsigned int cs_change_at_end) {
	int msgSize = sizeof(uint8_t) * 28 * (ledStructure->numLeds / 4);
	uint8_t *outputMsg = (uint8_t *) malloc(msgSize);
	memset(outputMsg, 0, msgSize);
	uint8_t *inputMsg = (uint8_t *) malloc(msgSize);
	memset(inputMsg, 0, msgSize);
	for (int i = 0; i < ledStructure->numLeds; i = i + 4) {
		constructPacketRGB(ledStructure->settings, ledStructure->brightness, ledStructure->individualLeds + i, outputMsg + i * 7);
	}
	printHexBuffer(outputMsg, msgSize);
	spi_duplexTransfer(spi_device, outputMsg, inputMsg, 28 * (ledStructure->numLeds / 4), speed, bits_per_word, cs_change_at_end);

	free(outputMsg);
	free(inputMsg);
}

void printLedStructure(struct allLedControlStruct *ledStructure) {
	printf("Num leds %d\nGlobal brightness ( %d, %d, %d)\n", ledStructure->numLeds, ledStructure->brightness.rBrightness, ledStructure->brightness.gBrightness, ledStructure->brightness.bBrightness);
	for (int i = 0; i < ledStructure->numLeds; i++) {
		printf(" + Led %2d: (%X, %X, %X)\n", i, ledStructure->individualLeds[i].r, ledStructure->individualLeds[i].g, ledStructure->individualLeds[i].b);
	}
}

void printHexBuffer(void *buffer, int length) {
	for (int i = 0; i < length; i++) {
		printf("0x%X ", *((uint8_t *)(buffer+i)));
	}
	printf("\n");
}

void destroyLedControll(struct allLedControlStruct *ledStructure) {
	free(ledStructure->individualLeds);
	free(ledStructure);
}
