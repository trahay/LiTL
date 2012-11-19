
Home = ..
LIBS = -lrt

CC = gcc
CFLAGS = -Wall -O2 -fPIC

OBJ_LIB = evnt_write.o evnt_read.o evnt_macro.o
OBJ_MAIN = main.o evnt_write.o evnt_read.o evnt_macro.o

export USE_GETTID=1

all: clean main.out static dynamic
static: libevnt.a
dynamic: libevnt.so

libevnt.a: $(OBJ_LIB)
	ar rcv $@ $(OBJ_LIB)

libevnt.so: $(OBJ_LIB)
	$(CC) $(OBJ_LIB) $(LIBS) -shared -o $@

main.out: $(OBJ_MAIN)
	$(CC) $(OBJ_MAIN) $(LIBS) -o $@

main.o: main.c evnt_types.h evnt_write.h evnt_read.h
evnt_read.o: evnt_read.c evnt_read.h evnt_types.h evnt_macro.h
evnt_write.o: evnt_write.c evnt_write.h evnt_types.h evnt_macro.h
evnt_macro.o: evnt_macro.c evnt_macro.h

.PHONY: clean
clean:
	rm -f *.o *.out
	rm -f libevnt.*
