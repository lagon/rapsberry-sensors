#include "i2clib.h"
#include <linux/i2c-dev.h>

#define I2CDATAPRINT(op, busAddr, reg, data) printf("%s address 0x%0X register 0x%0X data 0x%0X\n", op, busAddr, reg, data);
//#define I2CDATAPRINT(op, busAddr, reg, data)
#define I2CBULKDATAPRINT(op, busAddr, data, len) printf("%s, address: 0x%0X ", op, busAddr); for (int __i2cI = 0; __i2cI < len; __i2cI++) { printf(" 0x%0X", ((uint8_t *)data)[__i2cI]);}; printf("\n");

int i2c_initDevice(int bus_id) {
    char *busName;
    if (bus_id == 0) {
        busName = "/dev/i2c-0";
    } else if (bus_id == 1) {
        busName = "/dev/i2c-1";
    } else {
        errno = ENODEV;
        return -1;
    }

    int fd;
    if ((fd = open(busName, O_RDWR, 0)) < 0) {					// Open port for reading and writing
        printf("Failed to open i2c port\n");
        return -1;
    }

    return fd;
}

int selectDevice(int fd, uint8_t address) {
    if (address > 127) {
        errno = EADDRNOTAVAIL;
        return -2;
    }

    if (ioctl(fd, I2C_SLAVE, address) < 0) {                    // Set the port options and set the address of the device we wish to speak to
        perror("");
        printf("Unable to get bus access to talk to slave\n");
        return -1;
    }
    return 0;
}

void printHexDump(char *op, uint8_t addr, void *data, uint8_t length) {
    printf("%s from %0X: ", op, addr);
    for (int i = 0; i < length; i++) {
        printf("%0X ", (*(uint8_t *)(data + i)));
    }
    printf("\n");
}

int i2c_writeToDevice(int fd, uint8_t address, void *data, uint8_t length) {
    selectDevice(fd, address);
    I2CBULKDATAPRINT("Write", address, data, length);
    int written = write(fd, data, length);
    return written;
}

int i2c_readFromDevice(int fd, uint8_t address, void *data, uint8_t length) {
    selectDevice(fd, address);
    int readBytes = read(fd, data, length);
    I2CBULKDATAPRINT("Read", address, data, length);
    return readBytes;    
}

uint16_t combineBytesToWord(uint8_t msb, uint8_t lsb) {
    uint16_t output = (msb << 8) + lsb;
    return output;
}

int i2c_read16bitsWithRetry(int fd, uint8_t address, uint8_t reg, uint16_t *readData, uint8_t retries) {
    int ret = -10;
    while ((ret < 0) && (retries > 0)) {
        ret = i2c_read16bits(fd, address, reg, readData);
        retries--;
    }
    return ret;
}

int i2c_read16bits(int fd, uint8_t address, uint8_t reg, uint16_t *readData) {
    if (selectDevice(fd, address) < 0) {
        return -1;
    }
    __s32 ret = i2c_smbus_read_word_data(fd, reg);
    if (ret < 0) {
        //perror("Unable to read 16 bits from i2c");
        return -2;
    }
    *readData = ret;// & 0x0000FFFF;
    I2CDATAPRINT("Read from", address, reg, *readData);
    return 1;
}

int i2c_write16bits(int fd, uint8_t address, uint8_t reg, uint16_t value) {
    if (selectDevice(fd, address) < 0) {
        return -1;
    }
    if (i2c_smbus_write_word_data(fd, reg, value) < 0) {
        //perror("Unable to write 16 bits from i2c");
        return -1;
    }
    I2CDATAPRINT("Write to", address, reg, value);
    return 1;
}

int i2c_read8bitsWithRetry(int fd, uint8_t address, uint8_t reg, uint8_t *readData, uint8_t retries) {
    int ret = -10;
    while ((ret < 0) && (retries > 0)) {
        ret = i2c_read8bits(fd, address, reg, readData);
        retries--;
    }
    return ret;
}


int i2c_read8bits(int fd, uint8_t address, uint8_t reg, uint8_t *readData) {
    if (selectDevice(fd, address) < 0) {
        return -1;
    }

    __s32 ret = i2c_smbus_read_byte_data(fd, reg);
    if (ret < 0) {
        //perror("Unable to read 8 bits from i2c");
        return -2;
    }
    *readData = ret;// & 0x000000FF;
    I2CDATAPRINT("Read from", address, reg, *readData);
    return 1;
}

int i2c_write8bits(int fd, uint8_t address, uint8_t reg, uint8_t value) {
    if (selectDevice(fd, address) < 0) {
        return -1;
    }

    if (i2c_smbus_write_byte_data(fd, reg, value) < 0) {
        //perror("Unable to write 8 bits from i2c");
        return -1;
    }
    I2CDATAPRINT("Write to", address, reg, value);
    return 1;
}

void i2c_closeDevice(int fd) {
    close(fd);
}