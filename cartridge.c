#include "common.h"
#include "cartridge.h"
#include "cpu.h"

static nes_header header = { 0 };
static nes_mapper mapper = { 0 };

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

void parse_header(uint8_t* rom) {
	
	if (rom[0] != 'N' || rom[1] != 'E' || rom[2] != 'S' || rom[3] != 0x1A) {
		printf("File is not a nes file\n");
		exit(EXIT_FAILURE);
	}

	if (((rom[7] >> 2)  & 0x3) == 0x2) {
		printf("File is a NES 2.0 file\n");
	}
	
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
}

void init_mapper(uint8_t* rom) {

	if (header.trainer) {
		mapper.rom = rom + HEADER_SIZE + TRAINER_SIZE;
		printf("Ignoring trainer.\n");
	} else {
		mapper.rom = rom + HEADER_SIZE;
	}

	switch (header.number) {

		uint8_t* prgram;

		case 0:
			prgram = calloc(0x2000, 1);
			if (!prgram) {
				perror("prgram calloc failed.\n");
				exit(EXIT_FAILURE);
			}
			mapper.prgram = prgram;

			break;

		default:
			printf("Mapper number %03d not supported.\n", header.number);
			exit(EXIT_FAILURE);
	}
}

uint16_t get_reset_vector() {

	uint8_t lo = cpu_read(0xfffc);
	uint8_t hi = cpu_read(0xfffd);

	return (hi << 8) | lo;
}

uint16_t get_irq_vector() {

	uint8_t lo = cpu_read(0xfffe);
	uint8_t hi = cpu_read(0xffff);

	return (hi << 8) | lo;
}

uint16_t get_nmi_vector() {

	uint8_t lo = cpu_read(0xfffa);
	uint8_t hi = cpu_read(0xfffb);

	return (hi << 8) | lo;
}

uint8_t mapper_read(uint16_t address) {

	uint8_t value;
	
	switch (header.number) {

		case 0:
			if (address >= 0x6000 && address <= 0x7fff) {
				value = mapper.prgram[address - 0x6000];

			} else if (address >= 0x8000 && address <= 0xbfff) {
				value = mapper.rom[address - 0x8000];

			} else if (address >= 0xc000 && address <= 0xffff) {
				if (header.prgrom == 1) {
					value = mapper.rom[address - 0xc000];
				} else if (header.prgrom == 2) {
					value = mapper.rom[address - 0x8000];
				} else {
					printf("Bad mapper!\n");
					exit(EXIT_FAILURE);
				}

			} else {
				printf("Read mapper at %02x not implemented.\n", address);
				exit(EXIT_FAILURE);
			}

			break;

		default:
			printf("Mapper number %03d not supported.\n", header.number);
			exit(EXIT_FAILURE);
	}

	return value;
}

void mapper_write(uint8_t value, uint16_t address) {

	switch (header.number) {

		case 0:
			if (address >= 0x6000 && address <= 0x7fff) {
				mapper.prgram[address - 0x6000] = value;

			} else if (address >= 0x8000 && address <= 0xffff) {
				printf("Write to PRG ROM at %02x is not legal (i think).\n", address);
				exit(EXIT_FAILURE);

			} else {
				printf("Write mapper at %02x not implemented.\n", address);
				exit(EXIT_FAILURE);
			}

			break;

		default:
			printf("Mapper number %03d not supported.\n", header.number);
			exit(EXIT_FAILURE);
	}	
}

uint8_t ppu_mapper_read(uint16_t address) {

	uint8_t value;

	switch(header.number) {

		case 0:
			value = mapper.rom[address + 0x4000 * header.prgrom];
			break;

		default:
			printf("Mapper number %03d not supported.\n", header.number);
			exit(EXIT_FAILURE);
	}

	return value;
}

void ppu_mapper_write(uint8_t value, uint16_t address) {

	switch(header.number) {

		case 0:
			mapper.rom[address + 0x4000 * header.prgrom] = value;
			break;

		default:
			printf("Mapper number %03d not supported.\n", header.number);
			exit(EXIT_FAILURE);
	}
}

uint8_t log_read_mapper(uint16_t address) {

	uint8_t value;

	switch (header.number) {

			case 0:
				if (address >= 0x6000 && address <= 0x7fff) {
					value = mapper.prgram[address - 0x6000];

				} else if (address >= 0x8000 && address <= 0xbfff) {
					value = mapper.rom[address - 0x8000];

				} else if (address >= 0xc000 && address <= 0xffff) {
					if (header.prgrom == 1) {
						value = mapper.rom[address - 0xc000];
					} else if (header.prgrom == 2) {
						value = mapper.rom[address - 0x8000];
					} else {
						printf("Bad mapper!\n");
						exit(EXIT_FAILURE);
					}

				} else {
					printf("Read mapper at %02x not implemented.\n", address);
					exit(EXIT_FAILURE);
				}

				break;

			default:
				printf("Mapper number %03d not supported.\n", header.number);
				exit(EXIT_FAILURE);
		}

		return value;
}

uint8_t debug_ppu_mapper_read(uint16_t address) {
	
	uint8_t value;

	switch(header.number) {

		case 0:
			value = mapper.rom[address + 0x4000 * header.prgrom];
			break;

		default:
			printf("Mapper number %03d not supported.\n", header.number);
			exit(EXIT_FAILURE);
	}

	return value;
}

void free_mapper(void) {

	free(mapper.prgram);
}

bool get_mirroring(void) {
	return header.mirroring;
}