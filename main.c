#include "common.h"
#include "cartridge.h"
#include "bus.h"
#include "cpu.h"

size_t cycles = 7;

int main(int argc, char* argv[argc+1]) {

	uint8_t* rom = read_rom(argv[1]);
	nes_header header = parse_header(rom);
	nes_mapper mapper = init_mapper(rom, header);

	nes_bus* bus = init_bus(mapper);
	nes_cpu* cpu = init_cpu(bus);
		
	for (size_t i = 0; i < 65; ++i) {
		clock_cpu(cpu, bus);
		++cycles;
	}

	free(rom);
	free(bus->mapper.prgram);
	free(bus->ram);
	free(bus);
	free(cpu);

	return EXIT_SUCCESS;
}

// Known errors:
//		improve makefile