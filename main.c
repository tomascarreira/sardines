#include "common.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"

size_t cycles = 7;

int main(int argc, char* argv[argc+1]) {

	uint8_t* rom = read_rom(argv[1]);
	parse_header(rom);
	init_mapper(rom);

	init_cpu();
	init_ram();

	init_ppu();
		
	for (size_t i = 0; i < 65; ++i) {
		clock_cpu(cpu, bus);
		++cycles;
	}

	free(rom);
	free_mapper();
	free_ram();

	return EXIT_SUCCESS;
}

// Known errors:
//		improve makefile, and header files