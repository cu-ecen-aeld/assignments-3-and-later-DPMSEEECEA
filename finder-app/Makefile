##--------------------------My Makefile ------------------------##
# -Wall, This enables all the warnings about constructions.
# -g, Debug Info (gdb)
# -c, assemble; do not link.
# -o, outut
# -I, include dir
##
#CROSS_COMPILE = aarch64-none-linux-gnu-
cc = $(CROSS_COMPILE)gcc
soruce := writer.c
target = writer
cflags = -g -Wall

$(target).o: $(soruce)
	$(cc) $(cflags) -c -o $(target).o $(soruce); $(cc) $(cflags) -I/ $(target).o -o $(target)
	
clean:
	-rm -f *.o $(target) 