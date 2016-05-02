include basic_defs.makefile

OBJS=   actions/h21df_action.o \
		actions/printSensorsStat_action.o \
		actions/sqlite_store_sensor_stat.o \
		actions/bmp183_action.o \
		actions/save2file_action.o \
		actions/ina219_action.o \
		actions/mcp9808_action.o \
		actions/kbInput_action.o \
		actions/led_driver_action.o \
		actions/mpr121_action.o \
		actions/sfr02_action.o

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

compile: $(OBJS)
