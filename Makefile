CC=gcc
CFLAGS=-Wall -Wpedantic -g
OBJS=
BIN=sardines

all:$(BIN)

sardines:$(OBJS)
	$(CC) $(CFLAGS) -o sardines

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r sardines *.o