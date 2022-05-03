CC=gcc
CFLAGS=-Wall -Wpedantic -g
OBJS=cartridge.o main.o bus.o
BIN=sardines

all:$(BIN)

sardines:$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o sardines

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r sardines *.o