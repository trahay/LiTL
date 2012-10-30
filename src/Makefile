
Home = ..
CC = gcc
CFLAGS = -Wall -O2

OBJ = main.o

main.out: $(OBJ)
	$(CC) $(OBJ) -o $@

main.o: main.c evnt_types.h evnt_write.h

.PHONY: clean
clean: 
	rm -f *.o *.out
