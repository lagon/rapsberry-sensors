INCLUDES=-I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include  -I./actions -I./sensor_libraries -I.
LIBS=-lglib-2.0 -lsqlite3 -lm

CC=gcc
CFLAGS=-g -O0 -Wall -std=gnu99 $(INCLUDES)
OUTPUTDIR=./output
