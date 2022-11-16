##--------------------------My Makefile ------------------------##
# -Wall, This enables all the warnings about constructions.
# -g, Debug Info (gdb)
# -c, assemble; do not link.
# -o, outut
# -I, include dir
##
#CROSS_COMPILE = aarch64-none-linux-gnu-
soruce := writer.c
target = writer
cflags = -g -Wall

ifdef CROSS_COMPILE
	cc = $(CROSS_COMPILE)gcc
else
	cc = gcc
endif

all: $(soruce)
	$(cc) $(cflags) -c -o $(target).o $(soruce); $(cc) $(cflags) -I/ $(target).o -o $(target)
	
clean:
	-rm -f *.o $(target) 