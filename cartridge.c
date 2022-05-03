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

nes_header parse_header(uint8_t* rom) {
	
	if (rom[0] != 'N' || rom[1] != 'E' || rom[2] != 'S' || rom[3] != 0x1A) {
		printf("File is not a nes file\n");
		exit(EXIT_FAILURE);
	}

	if (((rom[7] >> 2)  & 0x3) == 0x2) {
		printf("File is a NES 2.0 file\n");
	}

	nes_header header = { 0 };
	
	header.number = (rom[6] >> 4) || (rom[7] & 0xf0);
	header.prgrom = rom[4];
	header.chrrom = rom[5];
	header.prgram = rom[8];
	header.mirroring = rom[6] & 0x1;
	header.ignore_mirror = (rom[6] >> 3) & 0x1;
	header.battery = (rom[6] >> 1) & 0x1;
	header.trainer = (rom[6] >> 2) & 0x1;
	header.vs_unisystem = rom[7] & 0x1;
	header.play_choice_10 = (rom[7] >> 1) & 0x1;
	header.tv_system = rom[9] & 0x1;

	return header;
}

mapper init_mapper(uint8_t* rom, nes_header header) {

	mapper mapper = { 0 };

	switch (header.number) {

		case 0:
			mapper.header = header;
			mapper.rom = rom + HEADER_SIZE;
			break;

		default:
			printf("Mapper number %03d not supported.\n", header.number);
			exit(EXIT_FAILURE);
	}

	return mapper;
}