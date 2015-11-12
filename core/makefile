include basic_defs.makefile

OBJS=core/actionQueue.o core/main_event_loop.o core/sqliteDbUtilityFunctions.o core/actionDescriptorStructure.o core/sensorConfigParser.o core/jsmn.o

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

compile: $(OBJS)

