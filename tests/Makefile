
Home =  ..
LIBS = -lrt -levnt
INCLUDE = -I$(Home)/src
DIRS = -L$(Home)/src

CC = gcc
CFLAGS = -Wall -O2
CFLAGS += $(INCLUDE)

OBJ_TEST_WRITE = test_evnt_write.o
OBJ_TEST_READ = test_evnt_read.o
OBJ_TEST_BUFFER = test_evnt_buffer_size.o timer.o

all: clean test_evnt_write test_evnt_read test_evnt_buffer

test_evnt_write: $(OBJ_TEST_WRITE)
	$(CC) $(OBJ_TEST_WRITE) $(INCLUDE) $(DIRS) $(LIBS) -o $@
	
test_evnt_read: $(OBJ_TEST_READ)
	$(CC) $(OBJ_TEST_READ) $(INCLUDE) $(DIRS) $(LIBS) -o $@
	
test_evnt_buffer: $(OBJ_TEST_BUFFER)
	$(CC) $(OBJ_TEST_BUFFER) $(INCLUDE) $(DIRS) $(LIBS) -o $@

timer.o: timer.c timer.h
test_evnt_write.o: test_evnt_write.c
test_evnt_read.o: test_evnt_read.c
test_evnt_buffer_size.o: test_evnt_buffer_size.c timer.h

.PHONY: clean
clean: 
	rm -f *.o
	rm -f test_evnt_write
	rm -f test_evnt_read
	rm -f test_evnt_buffer
