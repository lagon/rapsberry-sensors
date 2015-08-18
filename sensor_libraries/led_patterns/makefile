include basic_defs.makefile

OBJS= sensor_libraries/led_patterns/all_led_patterns.o sensor_libraries/led_patterns/pattern_queue.o

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

compile: $(OBJS)

