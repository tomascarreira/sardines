#include "common.h"
#include "cartridge.h"
#include "bus.h"

int main(int argc, char* argv[argc+1]) {

	uint8_t* rom = read_rom(argv[1]);
	nes_header header = parse_header(rom);
	mapper mapper = init_mapper(rom, header);

	bus* bus = init_bus(mapper);

	return EXIT_SUCCESS;
}