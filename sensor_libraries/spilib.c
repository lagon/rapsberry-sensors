#include "spilib.h"

int spi_initDevice(int busID, int deviceID) {
    char busC;
    char devC;
    if (busID == 0) {
        busC = '0';
    } else if (busID == 1) {
        busC = '1';
    } else {
        errno = ENODEV;
        return -1;
    }

    if (deviceID == 0) {
        devC = '0';
    } else if (deviceID == 1) {
        devC = '1';
    } else {
        errno = ENODEV;
        return -2;
    }

    char *devFilename = (char *)malloc(sizeof(char) * 128);
    if (snprintf(devFilename, 128, "/dev/spidev%c.%c", busC, devC) < 0) {
        syslog(LOG_ERR, "Unable to produce SPI bus name");
        return -3;
    }

    int fd = open(devFilename, O_RDWR);
    if (fd < 0) {
        int xerrno = errno;
        char *errorStr = strerror(xerrno);
        char *msg = (char *)malloc(sizeof(char) * 1024);
        snprintf(msg, 1024, "Error opening the SPI bus device file %s. Error was %s", devFilename, errorStr);
        syslog(LOG_ERR, msg);
        free(devFilename);
        free(msg);
        return -4;
    }

    free(devFilename);
    return fd;
}

int spi_read8bFromAnyAddress(int device, const uint8_t *address, uint8_t addressLen, uint8_t *retValue, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end) {
    uint8_t *out_data = (uint8_t *) malloc(sizeof(uint8_t) * (addressLen + 1));
    uint8_t *in_data = (uint8_t *) malloc(sizeof(uint8_t) * (addressLen + 1));
    memset(out_data, 0, addressLen + 1);
    memset(in_data, 0, addressLen + 1);

    memcpy(out_data, address, addressLen);
    out_data[0] = out_data[0] | 0x80;

    if (spi_duplexTransfer(device, out_data, in_data, addressLen + 1, speed, bits_per_word, cs_change_at_end) < 0) {
        free(out_data);
        free(in_data);

        return -1;
    }

    *retValue = in_data[addressLen];
    free(out_data);
    free(in_data);

    return 1;
}

int spi_read16bFromAnyAddress(int device, const uint8_t *address, uint8_t addressLen, uint16_t *retValue, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end) {
    uint8_t *out_data = (uint8_t *) malloc(sizeof(uint8_t) * (addressLen + 2));
    uint8_t *in_data = (uint8_t *) malloc(sizeof(uint8_t) * (addressLen + 2));
    memset(out_data, 0, addressLen + 2);
    memset(in_data, 0, addressLen + 2);

    memcpy(out_data, address, addressLen);
    // out_data[0] = out_data[0] | 0x80;

    if (spi_duplexTransfer(device, out_data, in_data, addressLen + 2, speed, bits_per_word, cs_change_at_end) < 0) {
        free(out_data);
        free(in_data);
        return -1;
    }

    uint16_t r = (in_data[addressLen] << 8) + in_data[addressLen+1];
    *retValue = r;
    free(out_data);
    free(in_data);
    return 1;
}

int spi_readBytesFromAnyAddress(int device, const uint8_t *address, uint8_t addressLen, uint8_t *retData, uint32_t retDataLen, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end) {
    uint8_t *out_data = (uint8_t *) malloc(sizeof(uint8_t) * (addressLen + retDataLen));
    uint8_t *in_data = (uint8_t *) malloc(sizeof(uint8_t) * (addressLen + retDataLen));
    memset(out_data, 0, addressLen + retDataLen);
    memset(in_data, 0, addressLen + retDataLen);

    memcpy(out_data, address, addressLen);
    // out_data[0] = out_data[0] | 0x80;

    if (spi_duplexTransfer(device, out_data, in_data, addressLen + retDataLen, speed, bits_per_word, cs_change_at_end) < 0) {
        free(out_data);
        free(in_data);

        return -1;
    }

    memcpy(retData, in_data + addressLen, retDataLen);

    free(out_data);
    free(in_data);

    return 1;
}

int spi_write8bToAnyAddress(int device, const uint8_t *address, uint8_t addressLen, uint8_t value, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end) {
    uint8_t *out_data = (uint8_t *) malloc(sizeof(uint8_t) * (addressLen + 1));
    uint8_t *in_data = (uint8_t *) malloc(sizeof(uint8_t) * (addressLen + 1));
    memset(out_data, 0, addressLen + 1);
    memset(in_data, 0, addressLen + 1);

    memcpy(out_data, address, addressLen);
    // out_data[0] = out_data[0] & 0x7F;
    out_data[addressLen] = value;

    if (spi_duplexTransfer(device, out_data, in_data, addressLen + 1, speed, bits_per_word, cs_change_at_end) < 0) {
        return -1;
    }

    free(out_data);
    free(in_data);
    return 1;
}


int spi_duplexTransfer(int device, uint8_t *out_data, uint8_t *in_data, unsigned int length, unsigned int speed, unsigned int bits_per_word, unsigned int cs_change_at_end) {
    struct spi_ioc_transfer *spi = (struct spi_ioc_transfer *) malloc(sizeof(struct spi_ioc_transfer) * length);
    if (spi == NULL) {perror("Unable to allocate memory"); exit(-1);}
    for (int i = 0; i < length; i++) {
        spi[i].tx_buf        = (unsigned long)(out_data + i);
        spi[i].rx_buf        = (unsigned long)(in_data + i);
        spi[i].len           = 1;
        spi[i].delay_usecs   = 0;
        spi[i].speed_hz      = speed;
        spi[i].bits_per_word = bits_per_word;
        spi[i].cs_change     = 0;
    }

    if (cs_change_at_end == 1) {
        spi[length-1].cs_change = 1;
    }
    if (ioctl (device, SPI_IOC_MESSAGE(length), spi) < 0) {
        int xerrno = errno;
        char *errorStr = strerror(xerrno);
        char *msg = (char *)malloc(sizeof(char) * 1024);
        snprintf(msg, 1024, "Error sending out %d bytes of data. Error was %s", length, errorStr);
        syslog(LOG_ERR, msg);
        free(msg);
        free(spi);
        return -1;
    }
    // printf(">>>> ");
    // for (int i = 0; i < length; i++) {
    //     printf("%2x ", out_data[i]);
    // }
    // printf("\n");
    // printf("<<<< ");
    // for (int i = 0; i < length; i++) {
    //     printf("%2x ", in_data[i]);
    // }
    // printf("\n");


    free(spi);
    return 1;
}

int spi_getTransferMode(int device) {
    uint8_t spiMode;
    if (ioctl(device, SPI_IOC_RD_MODE, &spiMode) < 0) {
        char *errorStr = strerror(errno);
        char *msg = (char *)malloc(sizeof(char) * 1024);
        snprintf(msg, 1024, "Unable to get SPI mode. Error was: %s", errorStr);
        syslog(LOG_ERR, msg);
        free(msg);        
        return -2;
    }
    return spiMode;
}

int spi_setTransferMode(int device, uint8_t mode) {
    if ((mode != SPI_MODE_0) &&
        (mode != SPI_MODE_1) &&
        (mode != SPI_MODE_2) &&
        (mode != SPI_MODE_3)) {
        char *msg = (char *)malloc(sizeof(char) * 1024);
        snprintf(msg, 1024, "Value %d is not valid SPI mode (has to be one of %d, %d, %d, %d).", mode, SPI_MODE_0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3);
        syslog(LOG_ERR, msg);
        free(msg);
        return -1;
    }
    if (ioctl(device, SPI_IOC_WR_MODE, &mode) < 0) {
        char *errorStr = strerror(errno);
        char *msg = (char *)malloc(sizeof(char) * 1024);
        snprintf(msg, 1024, "Unable to set SPI mode %d. Error was: %s", mode, errorStr);
        syslog(LOG_ERR, msg);
        free(msg);        
        return -2;
    }
    return 1;
}

int spi_closeDevice(int device) {
    return close(device);
}
