CC=gcc
CFLAGS=-Wall -Wpedantic -g `sdl2-config --cflags --libs`
OBJS=cartridge.o main.o cpu.o ppu.o sdl.o input.o log.o
BIN=sardines

all:$(BIN)

sardines:$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o sardines

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r sardines *.o
