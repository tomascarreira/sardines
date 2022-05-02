#ifndef CARTRIDGE_H
#define CARTRIDGE_H

typedef struct nes_header nes_header;
struct nes_header {
	uint8_t mapper;
	uint8_t prgrom;
	uint8_t chrrom;
	uint8_t prgram;
	uint8_t mirroring; // 0 for horizontal, 1 for vertical
	bool ignore_mirror;
	bool battery;
	bool trainer; // only exister on modified dumps of oficial ROM cartridges
	bool vs_unisystem;
	bool play_choice_10;
	uint8_t tv_system; // 0 to NTSC, 1to PAL
};

uint8_t* read_rom(char* file_name);
nes_header parse_header(uint8_t* rom);

#endif