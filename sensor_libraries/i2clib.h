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

#define msleep(ms)     usleep((ms)*1000)

#ifndef __lagon_i2clib_h__

#define __lagon_i2clib_h__

int i2c_initDevice(int bus_id);

int i2c_writeToDevice(int fd, uint8_t address, void *data, uint8_t length);

int i2c_readFromDevice(int fd, uint8_t address, void *data, uint8_t length);

void i2c_closeDevice(int fd);

#endif