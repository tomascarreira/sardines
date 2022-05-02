#include "common.h"
#include "cartridge.h"

int main(int argc, char* argv[argc+1]) {

	uint8_t* rom = read_rom(argv[1]);

	nes_header header = parse_header(rom);

	return EXIT_SUCCESS;
}