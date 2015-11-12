#include "i2clib.h"

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
    if ((fd = open(busName, O_RDWR)) < 0) {					// Open port for reading and writing
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

int i2c_writeToDevice(int fd, uint8_t address, void *data, uint8_t length) {
    selectDevice(fd, address);
    int written = write(fd, data, length);
    return written;
}

int i2c_readFromDevice(int fd, uint8_t address, void *data, uint8_t length) {
    selectDevice(fd, address);
    int readBytes = read(fd, data, length);
    return readBytes;    
}

uint16_t combineBytesToWord(uint8_t msb, uint8_t lsb) {
    uint16_t output = (msb << 8) + lsb;
    return output;
}

uint16_t i2c_read16bits(int fd, uint8_t address, uint8_t reg) {
    uint8_t in_data[2] = {0, 0};
    if (i2c_writeToDevice(fd, address, &reg, 1) != 1) {
        perror("");
        syslog(LOG_ERR, "I2C - Unable to send out register address to read from");
        return 0xFFFF;
    };
    if (i2c_readFromDevice(fd, address, in_data, 2) != 2) {
        perror("");
        syslog(LOG_ERR, "I2C - Unable to get value");
        return 0xFFFF;
    }
    uint16_t rawValue;
    rawValue = combineBytesToWord(in_data[0], in_data[1]);  
    return rawValue;
}

int i2c_write16bits(int fd, uint8_t address, uint8_t reg, uint16_t value) {
    uint8_t out_data [3];
    out_data[0] = reg;
    out_data[1] = (value & 0xFF00) >> 8;
    out_data[2] = value & 0x00FF;

    if (i2c_writeToDevice(fd, address, out_data, 3) != 3) {
        perror("");
        syslog(LOG_ERR, "I2C - Unable write 16 bits of data to device");        
        return -1;
    }
    return 3;
}

int i2c_write8bits(int fd, uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t out_data [2];
    out_data[0] = reg;
    out_data[1] = value;

    if (i2c_writeToDevice(fd, address, out_data, 2) != 2) {
        perror("");
        syslog(LOG_ERR, "I2C - Unable write 8 bits of data to device");        
        return -1;
    }
    return 2;
}

void i2c_closeDevice(int fd) {
    close(fd);
}