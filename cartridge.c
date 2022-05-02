#include "common.h"
#include "cartridge.h"

uint8_t* read_rom(char* file_name) {

	FILE* f = fopen(file_name, "rb");
	if (!f) {
		perror("fopen failed\n");
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t* rom = malloc(size);

	if (!rom) {
		perror("rom malloc failed\n");
		exit(EXIT_FAILURE);
	}

	fread(rom, size, 1, f);
	fclose(f);

	return rom;
}