##--------------------------My Makefile ------------------------##
# -Wall, This enables all the warnings about constructions.
# -g, Debug Info (gdb)
# -c, assemble; do not link.
# -o, output
# -I, include dir
# making changes 
##
#CROSS_COMPILE = aarch64-none-linux-gnu-
soruce := writer.c
target = writer

CROSS_COMPILE = 
all: compile

compile:
	$(CROSS_COMPILE )$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(target) $(soruce) $(LDFLAGS)
#	$(cc) $(cflags) -c -o $(target).o $(soruce); $(cc) $(cflags) -I/ $(target).o -o $(target)
	
clean:
	-rm -f *.o $(target) 