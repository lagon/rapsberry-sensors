#include "i2clib.h"
#include <linux/i2c-dev.h>

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

    if (ioctl(fd, I2C_SLAVE, address) < 0) {					// Set the port options and set the address of the device we wish to speak to
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
    //printHexDump("Write", address, data, length);
    int written = write(fd, data, length);
    return written;
}

int i2c_readFromDevice(int fd, uint8_t address, void *data, uint8_t length) {
    selectDevice(fd, address);
    int readBytes = read(fd, data, length);
    //printHexDump("Read", address, data, readBytes);
    return readBytes;    
}

uint16_t combineBytesToWord(uint8_t msb, uint8_t lsb) {
    uint16_t output = (msb << 8) + lsb;
    return output;
}

int i2c_read16bits(int fd, uint8_t address, uint8_t reg, uint16_t *readData) {
    if (selectDevice(fd, address) < 0) {
        return -1;
    }
    __s32 ret = i2c_smbus_read_word_data(fd, reg);
    if (ret < 0) {
        return -2;
    }
    *readData = ret & 0x0000FFFF;
    return 1;
}

uint8_t i2c_read8bits(int fd, uint8_t address, uint8_t reg) {
    uint8_t in_data = 0;
    if (i2c_writeToDevice(fd, address, &reg, 1) != 1) {
        perror("");
        syslog(LOG_ERR, "I2C - Unable to send out register address to read from");
                return 0xFF;
    };
    usleep(2);
    if (i2c_readFromDevice(fd, address, &in_data, 1) != 1) {
        perror("");
        syslog(LOG_ERR, "I2C - Unable to get value");
        return 0xFF;
    }
    return in_data;
}

uint8_t i2c_read8bits(int fd, uint8_t address, uint8_t reg) {
    uint8_t in_data = 0;
    if (i2c_writeToDevice(fd, address, &reg, 1) != 1) {
        perror("");
        syslog(LOG_ERR, "I2C - Unable to send out register address to read from");
        return 0xFFFF;
    };
    if (i2c_readFromDevice(fd, address, &in_data, 1) != 1) {
        perror("");
        syslog(LOG_ERR, "I2C - Unable to get value");
        return 0xFFFF;
    }
    return in_data;
}

int i2c_write16bits(int fd, uint8_t address, uint8_t reg, uint16_t value) {
    if (selectDevice(fd, address) < 0) {
        return -1;
    }
    if (i2c_smbus_write_word_data(fd, reg, value) < 0) {
        return -1;
    } else {
        return 1;
    }
}

int i2c_read8bits(int fd, uint8_t address, uint8_t reg, uint8_t *readData) {
    if (selectDevice(fd, address) < 0) {
        return -1;
    }

    __s32 ret = i2c_smbus_read_byte_data(fd, reg);
    if (ret < 0) {
        return -2;
    }

    *readData = ret & 0x000000FF;
    return 1;
}

int i2c_write8bits(int fd, uint8_t address, uint8_t reg, uint8_t value) {
    if (selectDevice(fd, address) < 0) {
        return -1;
    }

    if (i2c_smbus_write_byte_data(fd, reg, value) < 0) {
        return -1;
    } else {
        return 1;
    }
}

void i2c_closeDevice(int fd) {
    close(fd);
}