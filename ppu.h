#ifndef PPU_H
#define PPU_H

typedef struct nes_ppuctrl nes_ppuctrl;
struct nes_ppuctrl {
	uint8_t nt_addr:2;
	uint8_t inc:1;
	uint8_t spr_addr:1;
	uint8_t bck_addr:1;
	uint8_t spr_size:1;
	uint8_t master_slave:1;
	uint8_t nmi:1;
};

typedef struct nes_ppumask nes_ppumask;
struct nes_ppumask {
	uint8_t greyscale:1;
	uint8_t bck_left:1;
	uint8_t spr_left:1;
	uint8_t background:1;
	uint8_t sprites:1;
	uint8_t emph_red:1;
	uint8_t emph_green:1;
	uint8_t emph_blue:1;
};

typedef struct nes_ppustatus nes_ppustatus;
struct nes_ppustatus {
	uint8_t last_write:5;
	uint8_t spr_overflow:1;
	uint8_t spr_0hit:1;
	uint8_t v_blank:1;
};

void init_ppu(void);
void clock_ppu(void);
uint8_t ppu_read(uint16_t address);
void ppu_write(uint8_t value, uint16_t address);
uint8_t ppu_registers_read(uint16_t address);
void ppu_registers_write(uint8_t value, uint16_t address);
uint8_t debug_ppu_read(uint16_t address);
void oam_write(uint8_t value, uint8_t address);
uint8_t colapse_ppustatus(void);
size_t get_ppu_cycle(void);
size_t get_scanline(void);
void increment_horizontal(void);
void increment_vertical(void);

#endif