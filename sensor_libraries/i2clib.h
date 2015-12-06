#include <inttypes.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>

#define msleep(ms)     usleep((ms)*1000)

#ifndef __lagon_i2clib_h__

#define __lagon_i2clib_h__

int i2c_initDevice(int bus_id);

int i2c_writeToDevice(int fd, uint8_t address, void *data, uint8_t length);

int i2c_readFromDevice(int fd, uint8_t address, void *data, uint8_t length);

uint16_t i2c_read16bits(int fd, uint8_t address, uint8_t reg);
uint8_t i2c_read8bits(int fd, uint8_t address, uint8_t reg);

int i2c_write16bits(int fd, uint8_t address, uint8_t reg, uint16_t value);
int i2c_write8bits(int fd, uint8_t address, uint8_t reg, uint8_t value);

void i2c_closeDevice(int fd);

#endif