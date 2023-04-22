#ifndef PPU_H
#define PPU_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define OAM_SPRITE_NUMBER 64
#define SECONDARY_OAM_SPRITE_NUMBER 8

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

typedef struct spr_attr spr_attr;
struct spr_attr {
	uint8_t pallet:2;
	uint8_t unimplemented:3;
	uint8_t priority:1;
	uint8_t flip_h:1;
	uint8_t flip_v:1;
};

typedef struct sprite sprite;
struct sprite {
	uint8_t top_y_pos;
	uint8_t tile_idx;
	spr_attr attributes;
	uint8_t left_x_pos;
};

// The three unimplemented bits of each sprite's byte 2 do not exist in the PPU and always read back as 0 on PPU revisions that allow reading PPU OAM through OAMDATA ($2004). This can be emulated by ANDing byte 2 with $E3 either when writing to or when reading from OAM. from nesdev.org

void init_ppu(void);
void clock_ppu(void);
uint8_t ppu_read(uint16_t address);
void ppu_write(uint8_t value, uint16_t address);
uint8_t ppu_registers_read(uint16_t address);
void ppu_registers_write(uint8_t value, uint16_t address);
uint8_t debug_ppu_read(uint16_t address);
sprite debug_oam_read(size_t i);
void oam_write(uint8_t value, uint8_t address);
uint8_t colapse_ppustatus(void);
size_t get_ppu_cycle(void);
size_t get_scanline(void);
void increment_horizontal(void);
void increment_vertical(void);

bool sprite_in_scanline(size_t scanline, uint8_t spr_y_pos);
uint8_t spr_attr_to_byte(spr_attr attr);
uint8_t pattern_table_encode_address(uint8_t tile_idx, uint section, uint y_offset, uint plane, uint spr_size);
void debug_oam(void);
void debug_secondary_oam(void);
nes_ppuctrl get_ppuctrl(void);

#endif

