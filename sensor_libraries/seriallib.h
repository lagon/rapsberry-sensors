#ifndef __lagon_serial_lib_h__
#define __lagon_serial_lib_h__

#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern const int serial_NonBlockingPort;
extern const int serial_BlockingPort;

int serial_openPort(const char *portName);
int serial_setBlockingMode(int port, int state);
int serial_setSpeed(int port, speed_t speed);
int serial_read(int port, char *buffer, int bufferSize);
int serial_write(int port, const char *buffer, int bytesToWrite);
void serial_closePort(int port);

#endif
