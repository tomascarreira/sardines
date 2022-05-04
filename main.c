#include "common.h"
#include "cartridge.h"
#include "bus.h"
#include "cpu.h"

int main(int argc, char* argv[argc+1]) {

	uint8_t* rom = read_rom(argv[1]);
	nes_header header = parse_header(rom);
	mapper mapper = init_mapper(rom, header);

	bus* bus = init_bus(mapper);
	cpu* cpu = init_cpu(bus);

	size_t clocks = 0;
		
	for (size_t i = 0; i < 65; ++i) {
		clock_cpu(cpu, bus);
		++clocks;
	}

	return EXIT_SUCCESS;
}

// Known errors:
//		+1 cycle on addressing modes is happening in instructions that shoudnt happend
//		needs to free calloc and malloc pointers