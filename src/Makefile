
Home = ..
LIBS = -lrt

CC = gcc
CFLAGS = -Wall -O2

OBJ_MAIN = main.o evnt_write.o evnt_read.o
OBJ_TEST_WRITE = test_evnt_write.o evnt_write.o

all: clean main.out

main.out: $(OBJ_MAIN)
	$(CC) $(OBJ_MAIN) $(LIBS) -o $@

test_evnt_write: $(OBJ_TEST_WRITE)
	$(CC) $(OBJ_TEST_WRITE) $(LIBS) -o $@

main.o: main.c evnt_types.h evnt_write.h
evnt_read.o: evnt_read.c evnt_read.h evnt_types.h
evnt_write.o: evnt_write.c evnt_write.h evnt_types.h  
test_evnt_write.o: test_evnt_write.c evnt_types.h evnt_write.h

.PHONY: clean
clean: 
	rm -f *.o *.out test_evnt_write
