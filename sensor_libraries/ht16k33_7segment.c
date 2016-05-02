#include <ht16k33.h>
#include <ht16k33_7segment.h>
#include <math.h>

		uint16_t _digits [] = {
/*000*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*005*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*010*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*015*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*020*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*025*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*030*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*035*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*040*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*045*/		0x0000, 0x0000, 0x0000, 0x003F, 0x0006,
/*050*/		0x005B, 0x004F, 0x0066, 0x006D, 0x007D,
/*055*/		0x0007, 0x007F, 0x006F, 0x0000, 0x0000,
/*060*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*065*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*070*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*075*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*080*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*085*/		0x0000, 0x0000, 0x0000, 0x0076, 0x0000,
/*090*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*095*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*100*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*105*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*110*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*115*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*120*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*125*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*130*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*135*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*140*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*145*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*150*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*155*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*160*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*165*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*170*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*175*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*180*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*185*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*190*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*195*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*200*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*205*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*210*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*215*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*220*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*225*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*230*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*235*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*240*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*245*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*250*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
/*255*/		0x0000, 0x0000, 0x0000, 0x0000, 0x0000};








		 //    0x003F, 0x1200, 0x00DB, 0x008F, 0x12E0, 0x00ED, 0x00FD, 0x0C01, 0x00FF, 0x00EF, // 0-9
		 //    0x00F7, 0x128F, 0x0039, 0x120F, 0x0079, 0x0071, // A-F
		 //    0x00BD, 0x00F6, 0x1200, 0x001E, 0x2470, 0x0038, 0x0536, 0x2136, 0x003F, 0x00F3, // G-P
		 //    0x203F, 0x20F3, 0x00ED, 0x1201, 0x003E, 0x0C30, 0x2836, 0x2D00, 0x1500, 0x0C09, // Q-Z
			// 0x1058, 0x2078, 0x00D8, 0x088E, 0x0858, 0x0C80, 0x048E, 0x1070, 0x1000, 0x000E, // a-j
			// 0x3600, 0x0030, 0x10D4, 0x1050, 0x00DC, 0x0170, 0x0486, 0x0050, 0x2088, 0x0078, // k-t
			// 0x001C, 0x2004, 0x2814, 0x28C0, 0x200C, 0x0848, // u-z
			// 0x0000, // blank
			// 0x0006, 0x0220, 0x12CE, 0x12ED, 0x0C24, 0x235D, 0x0400, 0x2400, 0x0900, 0x3FC0, 
   //          0x12C0, 0x0800, 0x00C0, 0x0000, 0x0C00 // Symbols
			// ]

struct ht16k33_7Segment *ht16k337Seg_initDevice(int bus_id, uint8_t address) {
	struct ht16k33_7Segment *dev = (struct ht16k33_7Segment *) malloc(sizeof(struct ht16k33_7Segment));
	dev->device = ht16k33_initDevice(bus_id, address);
 	if (dev->device == NULL) {
 		free(dev);
 		return NULL;
 	}
 	return dev;
}

int ht16k337Seg_printAscii(struct ht16k33_7Segment *device, uint8_t pos, uint8_t ch) {
	if (pos >= 2) {
		pos++;
	}
	return ht16k33_setBufferPosition(device->device, pos, _digits[ch]);
}

int ht16k337Seg_setPeriod(struct ht16k33_7Segment *device, uint8_t isShowing, uint8_t pos) {
	uint16_t value;
	if (pos >= 2) {
		pos++;
	}
	int ret = ht16k33_getBufferPosition(device->device, pos, &value);
	if (ret < 0) {return ret;}
	if (isShowing != 0) {
		value = value | 0x0080;
	} else {
		value = value | (~0x0080);
	}
	return ht16k33_setBufferPosition(device->device, pos, value);
}

int ht16k337Seg_setColon(struct ht16k33_7Segment *device, uint8_t isShowing) {
	if (isShowing != 0) {
		return(ht16k33_setBufferPosition(device->device, 3, 0x0001));
	} else {
		return(ht16k33_setBufferPosition(device->device, 3, 0x0000));
	}
}

int ht16k337Seg_printStrInternal(struct ht16k33_7Segment *device, char *string) {
	int len = strnlen(string, 4);
	len = len < 4 ? len : 4;
	for (int i = 0; i < len; i++) {
		int ret = ht16k337Seg_printAscii(device, i, string[i]);
		if (ret < 0) {return ret;}
	}

	return 0;	
}

int ht16k337Seg_printStr(struct ht16k33_7Segment *device, char *string) {
	int ret = ht16k337Seg_printStrInternal(device, string);
	if (ret < 0) {return ret;}

	return ht16k33_flushBufferToDisplay(device->device);
}

char * ht16k337Seg_intTo4Chars(int value) {
	char *str = malloc(sizeof(char) * 5);
	memset(str, 0, sizeof(char) * 5);
	if ((value < 0) || (value > 9999)) {
		strncpy(str, "XXXX", 4);
		return str;
	}
	for (int pos = 0; pos < 4; pos++) {
		int rem = value % 10;
		value = value / 10;
		str[3 - pos] = rem + 48;
	}
	return str;
}

int ht16k337Seg_printInteger(struct ht16k33_7Segment *device, int value) {
	char *str = ht16k337Seg_intTo4Chars(value);
	int ret = ht16k337Seg_printStr(device, str);
	free(str);
	return ret;
}

int ht16k337Seg_printDouble(struct ht16k33_7Segment *device, double value) {
	char *str = malloc(sizeof(char) * 4);
	int decimalPt = 0;

	if ((value < 0.0) || (value > 9999.0)) {
		strncpy(str, "XXXX", 4);
	} else {
		if ((value >= 10) && (value < 100))  {
			decimalPt = 1;
		} else if ((value >= 100) && (value < 1000)) {
			decimalPt = 2;
		} else if (value >= 1000) {
			decimalPt = 3;
		}
		int value10 = (int) round(exp10(3.0 - (double)decimalPt) * (double)value);
		str = ht16k337Seg_intTo4Chars(value10);
		printf("%g -> %d - %d - %s\n", value, value10, decimalPt, str);
	}

	ht16k337Seg_printStrInternal(device, str);
	ht16k337Seg_setPeriod(device, 1, decimalPt);
	return ht16k33_flushBufferToDisplay(device->device);
}

int ht16k337Seg_turnOn(struct ht16k33_7Segment *device) {
	return ht16k33_turnOn(device->device);
}

int ht16k337Seg_turnOff(struct ht16k33_7Segment *device) {
	return ht16k33_turnOff(device->device);
}

int ht16k337Seg_powerOn(struct ht16k33_7Segment *device) {
	return ht16k33_powerOn(device->device);
}

int ht16k337Seg_powerOff(struct ht16k33_7Segment *device) {
	return ht16k33_powerOff(device->device);
}

int ht16k337Seg_setBrightness(struct ht16k33_7Segment *device, uint8_t brightnessLevel) {
	return ht16k33_setBrightness(device->device, brightnessLevel);
}

int ht16k337Seg_setBlinking(struct ht16k33_7Segment *device, blinkState_t blinkState) {
	return ht16k33_setBlinking(device->device, blinkState);
}

void ht16k337Seg_closeDevice(struct ht16k33_7Segment *device) {
	ht16k33_closeDevice(device->device);
	free(device);
}

void ht16k337Seg_testDisplay() {
	struct ht16k33_7Segment * dev = ht16k337Seg_initDevice(1, 0x70);	
	ht16k337Seg_turnOff(dev);
	ht16k337Seg_powerOff(dev);
	sleep(1);

	ht16k337Seg_powerOn(dev);
	ht16k337Seg_turnOn(dev);
	ht16k337Seg_setBlinking(dev, HT16K33_NO_BLINK);
	ht16k337Seg_setBrightness(dev, 15);

	char *str = "890X";
	ht16k337Seg_printStr(dev, str);
	sleep(1);
	
	for (int i = 1; i < 1000; i++) {
		ht16k337Seg_printDouble(dev, ((double)i) / 1000.0);
		usleep(1000);
	}
	for (int i = 100; i < 1000; i++) {
		ht16k337Seg_printDouble(dev, ((double)i) / 100.0);
		usleep(1000);
	}
	for (int i = 100; i < 10000; i++) {
		ht16k337Seg_printDouble(dev, ((double)i) / 1.0);
		usleep(1000);
	}

	ht16k337Seg_turnOff(dev);
	ht16k337Seg_powerOff(dev);

}
