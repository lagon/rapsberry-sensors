include basic_defs.makefile

#OBJS=actionQueue.o main.o h21df_action.o h21df_library.o i2clib.o printSensorsStat_action.o sqlite_store_sensor_stat.o bmp183_library.o spilib.o
OBJS=utilityFunctions.o main.o

all: clean mk_output_dir make_core make_sensors make_actions make_base sensor_runner

make_actions:
	make -f actions/makefile compile

make_sensors:
	make -f sensor_libraries/makefile compile

make_core:
	make -f core/makefile compile

make_base: $(OBJS)

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

gather_objs:
	find . -iname "*.o" -exec mv -v {} $(OUTPUTDIR) \;

sensor_runner: gather_objs
		$(CC) $(CFLAGS) $(LIBS) -o sensor_runner output/*.o

mk_output_dir:
	mkdir $(OUTPUTDIR)

touch_source_files:
	find . -iname "*.c" -exec touch {} \;

clean: touch_source_files
	rm -fv $(OUTPUTDIR)/*.o sensor_runner
	rm -fRv $(OUTPUTDIR)