#include "ssd1306_oled_display.h"


const uint8_t ssd1306_PowerOnCommand = 0xAE;
const uint8_t ssd1306_PowerOffCommand = 0xAF;


struct ssd1306Display *ssd1306_initDisplay(uint8_t i2cBusID, uint8_t i2cAddress) {
	int fd = i2c_initDevice(i2cBusID);
	if (fd < 0) {
		char *errorMsg = (char *) malloc(1024);
		snprintf(errorMsg, 1024, "Error opening I2C connection to OLed display. Message: %s\n", strerror(errno));
		syslog(LOG_ERR, errorMsg);
		free(errorMsg);
		return NULL;
	}
	struct ssd1306Display *display = (struct ssd1306Display*) malloc(sizeof(struct ssd1306Display));
	display->i2cAddress = i2cAddress;
	display->i2cBusDevice = fd;

	display->height			= 64;
	display->width			= 128;
	display->_pages			= 64 / 8;
	display->orientation	= 0;
	display->framebuffer	= (uint8_t **) malloc(sizeof(uint8_t *) * display->_pages);
	for (int page = 0; page < display->_pages; page++) {
		display->framebuffer[page] = (uint8_t *) malloc(sizeof(uint8_t) * display->width);
	}

	return display;
};

void ssd1306_displayPowerOn(struct ssd1306Display *display) {
	uint8_t cmd = ssd1306_PowerOnCommand;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);
	cmd = 0xA5;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);
	cmd = 0xA7;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);

}

void ssd1306_displayPowerOff(struct ssd1306Display *display) {
	uint8_t cmd = ssd1306_PowerOffCommand;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);
}

void ssd1306_setContrast(struct ssd1306Display *display, uint8_t contrast) {

}

void ssd1306_setDisplayDim(struct ssd1306Display *display, uint8_t isDim) {

}

void ssd1306_setColorsInverted(struct ssd1306Display *display, uint8_t invertColors) {

}

void ssd1306_setDisplayAddressesForFrameBufferCopy(struct ssd1306Display *display) {
	// data[0] = ssd1306_ColumnAddressCmd;
	// data[1] = 0;
	// data[2] = display->width;

}

void ssd1306_showFrameBuffer(struct ssd1306Display *display) {

}


void ssd1306_setOrientation(struct ssd1306Display *display, uint8_t orentation) {

}

void ssd1306_drawPixel(struct ssd1306Display *display, uint8_t x, uint8_t y, uint8_t color) {

}



void ssd1306_demo() {
	struct ssd1306Display *display = ssd1306_initDisplay(1, 0x3d);
	ssd1306_displayPowerOn(display);
	sleep(10);
	ssd1306_displayPowerOff(display);
}








