#include "seriallib.h"

const int serial_NonBlockingPort = 1;
const int serial_BlockingPort    = 2;


int serial_initialisePort(int port) {
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (port, &tty) != 0) {
    	close(port);
        return -1;
    }

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_cc[VTIME] = 2;            // 0.2 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (port, TCSANOW, &tty) != 0) {
    	close(port);
        return -1;
    }
    return port;
}

int serial_openPort(const char *portName) {
	int port = open (portName, O_RDWR | O_NOCTTY | O_SYNC);
	serial_setSpeed(port, B9600);
	serial_setBlockingMode(port, serial_NonBlockingPort);
	return serial_initialisePort(port);
}

int serial_setBlockingMode(int port, int state) {
    struct termios ttystate;
    //get the terminal state
    if (tcgetattr(port, &ttystate) != 0) {
    	return -1;
    }

    ttystate.c_cc[VTIME] = 2;            // 0.2 seconds read timeout

    if (state==serial_NonBlockingPort) {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 0;
    } else if (state==serial_BlockingPort) {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    return(tcsetattr(port, TCSANOW, &ttystate));
}

int serial_setSpeed(int port, speed_t speed) {
	struct termios tty;
    memset (&tty, 0, sizeof(struct termios));
    if (tcgetattr (port, &tty) != 0) {
    	return -1;
    }
    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);
    if(tcsetattr (port, TCSANOW, &tty) != 0) {
    	return -1;
    }
    return 0;
}

int serial_read(int port, char *buffer, int bufferSize) {
	return(read(port, buffer, bufferSize));
}

int serial_write(int port, const char *buffer, int bytesToWrite) {
	return(write(port, buffer, bytesToWrite));	
}

void serial_closePort(int port) {
	close(port);
}