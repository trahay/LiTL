
Home = ..
LIBS = -lrt -lm

CC = gcc
CFLAGS = -Wall -O2 -fPIC

OBJ_LIB = evnt_write.o evnt_read.o evnt_macro.o
OBJ_EVNT_READ = evnt_read.o evnt_macro.o

export USE_GETTID=1

all: clean evnt_read dynamic
static: libevnt.a
dynamic: libevnt.so

libevnt.a: $(OBJ_LIB)
	ar rcv $@ $(OBJ_LIB)

libevnt.so: $(OBJ_LIB)
	$(CC) $(OBJ_LIB) $(LIBS) -shared -o $@

evnt_read: $(OBJ_EVNT_READ)
	$(CC) $(OBJ_EVNT_READ) $(INCLUDE) $(DIRS) $(LIBS) -o $@

evnt_macro.o: evnt_macro.c evnt_macro.h evnt_types.h
evnt_read.o: evnt_read.c evnt_read.h evnt_types.h evnt_macro.h
evnt_write.o: evnt_write.c evnt_write.h evnt_types.h evnt_macro.h

.PHONY: clean
clean:
	rm -f *.o *.out
	rm -f libevnt.*
	rm -f evnt_read
