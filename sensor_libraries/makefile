include basic_defs.makefile

OBJS= sensor_libraries/h21df_library.o sensor_libraries/i2clib.o sensor_libraries/bmp183_library.o sensor_libraries/spilib.o sensor_libraries/led_driver_library.o sensor_libraries/led_experiments.o sensor_libraries/ina219_power_monitor.o sensor_libraries/ssd1306_oled_display.o

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

compile: $(OBJS)

