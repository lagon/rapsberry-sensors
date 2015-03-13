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
    int written = read(fd, data, length);
    return written;    
}

void i2c_closeDevice(int fd) {
    close(fd);
}