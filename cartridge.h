#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#define HEADER_SIZE 16
#define TRAINER_SIZE 512

typedef struct nes_mapper nes_mapper;
struct nes_mapper {
	uint8_t* rom;
	uint8_t* prgram;
	void* registers;
};

typedef struct nes_header nes_header;
struct nes_header {
	uint8_t number;
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
void parse_header(uint8_t* rom);
void init_mapper(uint8_t* rom);
uint16_t get_reset_vector();
uint16_t get_irq_vector();
uint16_t get_nmi_vector();
uint8_t mapper_read(uint16_t address);
void mapper_write(uint8_t value, uint16_t address);
uint8_t ppu_mapper_read(uint16_t address);
void ppu_mapper_write(uint8_t value, uint16_t address);
uint8_t log_read_mapper(uint16_t address);
void print_str_in_prgram(void);
void free_mapper(void);

bool get_mirroring(void);

#endif