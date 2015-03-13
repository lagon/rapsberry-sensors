#ifndef __lagon_led_driver_library_h__
#define __lagon_led_driver_library_h__

#include "spilib.h"

struct rgbData {
	uint16_t r;
	uint16_t b;
	uint16_t g;
} rgbData;

struct globalBrightness {
	uint8_t rBrightness;
	uint8_t gBrightness;
	uint8_t bBrightness;
} globalBrightness;

struct ledPWMSettings {
	uint8_t blank;
	uint8_t dsprpt;
	uint8_t tmgrst;
	uint8_t extgck;
	uint8_t outtmg;
} ledPWMSettings;

struct allLedControlStruct {
	uint8_t numLeds;
	struct ledPWMSettings settings;
	struct globalBrightness brightness;
	struct rgbData *individualLeds;
} allLedControlStruct;

uint8_t *constructPacketRGB(struct ledPWMSettings settings, struct globalBrightness brightness, struct rgbData rgb[4], uint8_t *out_data);

struct allLedControlStruct *initiateLEDControls(uint8_t numLeds);

void setLedSettings(struct allLedControlStruct *ledStructure, struct ledPWMSettings *settings);

void setGlobalBrightness(struct allLedControlStruct *ledStructure, uint8_t brightness);
void setGlobalBrightnessRGB(struct allLedControlStruct *ledStructure, const struct rgbData *rgb);

void setOneLedRGB(struct allLedControlStruct *ledStructure, uint8_t ledIndex, const struct rgbData *rgb);
void setOneGrayscaleLed(struct allLedControlStruct *ledStructure, uint8_t gsLedIndex, uint16_t intensity);
void setThreeGrayscaleLed(struct allLedControlStruct *ledStructure, uint8_t ledIndex, uint16_t intensity1, uint16_t intensity2, uint16_t intensity3);

void sendOutLedDataDefaults(struct allLedControlStruct *ledStructure, const int spi_device);
void sendOutLedData(struct allLedControlStruct *ledStructure, const int spi_device, const int speed, const unsigned int bits_per_word, const unsigned int cs_change_at_end);

void destroyLedControll(struct allLedControlStruct *ledStructure);

void printLedStructure(struct allLedControlStruct *ledStructure);

#endif