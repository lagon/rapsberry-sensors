#ifndef __lagon_spilib_h__
#define __lagon_spilib_h__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>

#include "spilib.h"


int spi_initDevice(int busID, int deviceID);
int spi_setTransferMode(int device, uint8_t mode);

int spi_read8bFromAnyAddress(int device, const uint8_t *address, uint8_t addressLen, uint8_t *retValue, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end);
int spi_read16bFromAnyAddress(int device, const uint8_t *address, uint8_t addressLen, uint16_t *retValue, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end);

int spi_readBytesFromAnyAddress(int device, const uint8_t *address, uint8_t addressLen, uint8_t *retData, uint32_t retDataLen, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end);


int spi_write8bToAnyAddress(int device, const uint8_t *address, uint8_t addressLen, uint8_t value, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end);

int spi_duplexTransfer(int device, uint8_t *out_data, uint8_t *in_data, unsigned int length, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end);

int spi_closeDevice(int device);

#endif
