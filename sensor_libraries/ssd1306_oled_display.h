#ifndef __lagon_ssd1306_oled_display_h__
#define __lagon_ssd1306_oled_display_h__

#include "i2clib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>


struct ssd1306Display {
	uint8_t height;
	uint8_t width;
	uint8_t _pages;
	uint8_t orientation;

	uint8_t **framebuffer;

	uint8_t i2cAddress;
	int i2cBusDevice;
} ssd1306Display;


struct ssd1306Display *ssd1306_initDisplay(uint8_t i2cBusID, uint8_t i2cAddress);

void ssd1306_displayPowerOn(struct ssd1306Display *display);
void ssd1306_displayPowerOff(struct ssd1306Display *display);
void ssd1306_setContrast(struct ssd1306Display *display, uint8_t contrast);
void ssd1306_setDisplayDim(struct ssd1306Display *display, uint8_t isDim);
void ssd1306_setColorsInverted(struct ssd1306Display *display, uint8_t invertColors);

void ssd1306_showFrameBuffer(struct ssd1306Display *display);


void ssd1306_setOrientation(struct ssd1306Display *display, uint8_t orentation);

void ssd1306_drawPixel(struct ssd1306Display *display, uint8_t x, uint8_t y, uint8_t color);

void ssd1306_demo();
#endif