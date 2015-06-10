include basic_defs.makefile

OBJS=core/actionQueue.o core/main_event_loop.o core/sqliteDbUtilityFunctions.o core/actionDescriptorStructure.o

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

compile: $(OBJS)

