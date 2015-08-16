#include "ssd1306_oled_display.h"


const uint8_t ssd1306_cmd_displayOff         = 0xAF;
const uint8_t ssd1306_cmd_memoryMode         = 0x20;
const uint8_t ssd1306_cmd_setHighColumn      = 0x10;
const uint8_t ssd1306_cmd_lowColumn          = 0x00;
const uint8_t ssd1306_cmd_setContrast        = 0x81;
const uint8_t ssd1306_cmd_setSegmentRemap    = 0xA0;
const uint8_t ssd1306_cmd_normalDisplay      = 0xA6;
const uint8_t ssd1306_cmd_setMultiplex       = 0xA8;
const uint8_t ssd1306_cmd_displayOnResume    = 0xA4;
const uint8_t ssd1306_cmd_setDisplayOffset   = 0xD3;
const uint8_t ssd1306_cmd_setDisplayClockDiv = 0xD5;
const uint8_t ssd1306_cmd_setPreCharge       = 0xD9;
const uint8_t ssd1306_cmd_setComPins         = 0xDA;
const uint8_t ssd1306_cmd_setVComDetect      = 0xDB;
const uint8_t ssd1306_cmd_setChargePump      = 0x8D;
const uint8_t ssd1306_cmd_displayOn          = 0xAF;


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
	uint8_t cmd[5] = {0,0,0,0,0};
	cmd[0] = ssd1306_cmd_displayOff;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);

	cmd[0] = ssd1306_cmd_memoryMode;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);

	cmd[0] = ssd1306_cmd_setHighColumn;
	cmd[1] = 0xB0;
	cmd[2] = 0xCB;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 3);

	cmd[0] = ssd1306_cmd_lowColumn;
	cmd[1] = 0x10;
	cmd[2] = 0x40;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 3);

	cmd[0] = ssd1306_cmd_setContrast;
	cmd[1] = 0x7F;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 2);

	cmd[0] = ssd1306_cmd_setSegmentRemap;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);

	cmd[0] = ssd1306_cmd_normalDisplay;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);

	cmd[0] = ssd1306_cmd_setMultiplex;
	cmd[1] = 0x3F;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 2);

	cmd[0] = ssd1306_cmd_displayOnResume;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);

	cmd[0] = ssd1306_cmd_setDisplayOffset;
	cmd[1] = 0x00;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 2);

	cmd[0] = ssd1306_cmd_setDisplayClockDiv;
	cmd[1] = 0xF0;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 2);

	cmd[0] = ssd1306_cmd_setPreCharge;
	cmd[1] = 0x22;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 2);

	cmd[0] = ssd1306_cmd_setComPins;
	cmd[1] = 0x12;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 2);

	cmd[0] = ssd1306_cmd_setVComDetect;
	cmd[1] = 0x20;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 2);

	cmd[0] = ssd1306_cmd_setChargePump;
	cmd[1] = 0x14;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 2);

	cmd[0] = ssd1306_cmd_displayOn;
	i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);


            // const.MEMORYMODE,
            // const.SETHIGHCOLUMN,      0xB0, 0xC8,
            // const.SETLOWCOLUMN,       0x10, 0x40,
            // const.SETCONTRAST,        0x7F,
            // const.SETSEGMENTREMAP,
            // const.NORMALDISPLAY,
            // const.SETMULTIPLEX,       0x3F,
            // const.DISPLAYALLON_RESUME,
            // const.SETDISPLAYOFFSET,   0x00,
            // const.SETDISPLAYCLOCKDIV, 0xF0,
            // const.SETPRECHARGE,       0x22,
            // const.SETCOMPINS,         0x12,
            // const.SETVCOMDETECT,      0x20,
            // const.CHARGEPUMP,         0x14,
            // const.DISPLAYON



	// uint8_t cmd = ssd1306_PowerOnCommand;
	// i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);
	// cmd = 0xA5;
	// i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);
	// cmd = 0xA7;
	// i2c_writeToDevice(display->i2cBusDevice, display->i2cAddress, &cmd, 1);

}

void ssd1306_displayPowerOff(struct ssd1306Display *display) {
	uint8_t cmd = ssd1306_cmd_displayOff;
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
	sleep(20);
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	struct ssd1306Display *display = ssd1306_initDisplay(1, 0x3d);
	ssd1306_displayPowerOn(display);
	sleep(20);
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	ssd1306_displayPowerOff(display);
}








