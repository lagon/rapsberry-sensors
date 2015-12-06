include basic_defs.makefile

OBJS= sensor_libraries/mpr121.o sensor_libraries/h21df_library.o sensor_libraries/i2clib.o sensor_libraries/bmp183_library.o sensor_libraries/spilib.o sensor_libraries/led_driver_library.o sensor_libraries/led_experiments.o sensor_libraries/ina219_power_monitor.o sensor_libraries/ssd1306_oled_display.o sensor_libraries/mcp9808_temperature.o sensor_libraries/mtk3339-gps.o sensor_libraries/seriallib.o

make_led_patterns:
		make -f sensor_libraries/led_patterns/makefile compile

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

compile: make_led_patterns $(OBJS)

