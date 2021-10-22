CC = gcc
CFLAGS = -g -Wall

all: runsim testsim

runsim: main.c config.h
	$(CC) $(CFLAGS) -o runsim main.c config.h

testsim: testsim.c config.h
	$(CC) $(CFLAGS) -o testsim testsim.c config.h

clean:
	rm -f *.o runsim testsim