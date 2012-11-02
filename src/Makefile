
Home = ..
LIBS = -lrt

CC = gcc
CFLAGS = -Wall -O2

OBJ = main.o evnt_write.o evnt_read.o

main.out: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $@

main.o: main.c evnt_types.h evnt_write.h
evnt_read.o: evnt_read.c evnt_read.h evnt_types.h
evnt_write.o: evnt_write.c evnt_write.h evnt_types.h

.PHONY: clean
clean: 
	rm -f *.o *.out
