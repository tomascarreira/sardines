#include "common.h"
#include "ppu.h"
#include "cpu.h"
#include "cartridge.h"
#include "sdl.h"

static nes_ppuctrl ppuctrl = { 0 };
static nes_ppumask ppumask = { 0 };
static nes_ppustatus ppustatus = { 0 };
static uint8_t oamaddr;
static uint8_t oamdata;
static uint8_t ppuscroll;
static uint8_t ppuaddr;
static uint8_t ppudata;

static uint16_t t_loopy;
static uint16_t v_loopy;
static uint8_t x_loopy;
static bool w_loopy;

static uint8_t* vram;
static uint8_t* pallet;
static uint8_t* oam;

static size_t frame = 1;
static size_t scanline = 0;
static size_t cycle = 21;

void init_ppu() {

	oamaddr = 0;
	oamdata = 0;
	ppuscroll = 0;
	ppuaddr = 0;
	ppudata = 0;
	t_loopy = 0;
	v_loopy = 0;
	x_loopy = 0;
	w_loopy = false;


	vram = calloc(0x800, 1);
	if (!vram) {
		perror("vram calloc failed.\n");
		exit(EXIT_FAILURE);
	}

	oam = calloc(0x100, 1);
	if (!oam) {
		perror("oam calloc failed.\n");
		exit(EXIT_FAILURE);
	}

	pallet = calloc(0x20, 1);
	if (!pallet) {
		perror("pallet calloc failed.\n");
		exit(EXIT_FAILURE);
	}
}

void clock_ppu(void) {

	static uint8_t bg_next_tile_id;
	static uint8_t bg_next_tile_attr;
	static uint8_t bg_next_tile_lo;
	static uint8_t bg_next_tile_hi;

	static uint16_t bg_shift_patt_lo;
	static uint16_t bg_shift_patt_hi;
	static uint16_t bg_shift_attr_lo;
	static uint16_t bg_shift_attr_hi;

	if (cycle == 0 && scanline == 0) {
		clear_screen();
	}

	if (frame % 2 && scanline == 0 && cycle == 0) {
		++cycle;
		return;
	}


	if ((ppumask.background == 1 || ppumask.sprites == 1) && (scanline <= 230 || scanline == 261)) {		

		if ((cycle >= 1 && cycle <= 256) || (cycle >= 321 && cycle <= 336)) {
			
			bg_shift_patt_lo <<= 1;
			bg_shift_patt_hi <<= 1;
			bg_shift_attr_lo <<= 1;
			bg_shift_attr_hi <<= 1;

		if (scanline != 261 && (cycle >= 1 && cycle <= 256)) {
			uint8_t bg_pixel_lo = (bg_shift_patt_lo >> (15 - x_loopy)) & 0x0001;
			uint8_t bg_pixel_hi = (bg_shift_patt_hi >> (15 - x_loopy)) & 0x0001;
			uint8_t bg_pixel = (bg_pixel_hi << 1) | bg_pixel_lo;
			
			uint16_t pallet_addr =((bg_shift_attr_hi >> (15 - x_loopy)) << 3) | ((bg_shift_attr_lo >> (15 - x_loopy)) << 2) | bg_pixel; 	
			uint8_t color = ppu_read(pallet_addr + 0x3f00);
			draw_pixel(cycle - 1, scanline, color);
		}

			switch (cycle % 8) {

				case 0:
					bg_shift_patt_lo = (bg_shift_patt_lo & 0xff00) | bg_next_tile_lo;
					bg_shift_patt_hi = (bg_shift_patt_hi & 0xff00) | bg_next_tile_hi;
					uint8_t pal_attr = bg_next_tile_attr;
					if (v_loopy & 0x0001) {
						pal_attr >>= 2;
					}
					if (v_loopy & 0x03e0) {
						pal_attr >>= 4;
					}
					
					bg_shift_attr_lo = ((bg_shift_attr_lo & 0xff00) | ((pal_attr & 0x01) ? 0xff : 0x00));
					bg_shift_attr_hi = ((bg_shift_attr_hi & 0xff00) | ((pal_attr & 0x02) ? 0xff : 0x00));
					increment_horizontal();
					break;				

				case 1:
					bg_next_tile_id = ppu_read((v_loopy & 0x0fff) | 0x2000);
					break;

				case 3:
					bg_next_tile_attr = ppu_read((v_loopy & 0xc00) | ((v_loopy >> 4) & 0x38) | ((v_loopy >> 2) & 0x07) | 0x23c0);
					break;

				case 5:
					bg_next_tile_lo = ppu_read(ppuctrl.bck_addr  << 12 | (bg_next_tile_id << 4) | (v_loopy >> 12));
					break;
			
				case 7:
					bg_next_tile_hi = ppu_read(ppuctrl.bck_addr  << 12 | (bg_next_tile_id << 4) | (v_loopy >> 12) | 0x0008);
					break;
			}

			if (cycle == 256) {
				increment_vertical();
			}
		}

		if (cycle == 257) {
			v_loopy = (v_loopy & 0x7be0) | (t_loopy & ~0x7be0);
		}
	}

	if (scanline == 239 && cycle == 256) {
		present_frame();
	}

	if (scanline == 241 && cycle == 1) {
		ppustatus.v_blank = 1;
		if (ppuctrl.nmi == 1) {
			nmi();
		}
	}

	if (scanline == 261 && cycle == 1) {
		ppustatus.v_blank = 0;
		ppustatus.spr_0hit = 0;
		ppustatus.spr_overflow = 0;
	}

	if (scanline == 261 && cycle  >= 280 && cycle <= 304 && (ppumask.background == 1 || ppumask.sprites == 1)) {
	
		v_loopy = (v_loopy & 0x041f) | (t_loopy & ~0x041f);
	}

	++cycle;
	if (cycle > 340) {
		cycle = 0;
		++scanline;
	}
	if (scanline > 261) {
		scanline = 0;
		++frame;
	
		draw_pattern_table();
		draw_pallets();
	}
}

uint8_t ppu_read(uint16_t address) {

	uint8_t value;

	address &= 0x3fff;
	if (address <= 0x1fff) {
		value = ppu_mapper_read(address);

	} else if (address >= 2000 && address <= 0x3eff) {
		address &= 0x2fff;
		if (get_mirroring()) { // vertical mirroring
			value = vram[address & 0x7ff];

		} else { // horizontal mirroring
			if (address <= 0x23ff) {
				value = vram[address & 0x3ff];
			} else  if (address <= 0x27ff) {
				value = vram[(address & 0x3ff) + 0x400];
			} else if (address <= 0x2bff) {
				value = vram[address & 0x3ff];
			} else if (address <= 0x2fff) {
				value = vram[(address & 0x3ff) + 0x400];
			} else {
				printf("Something went wrong reading vram at %04x", address);
			}

		}

	} else if (address >= 0x3f00 && address <= 0x3fff) {
		address &= 0x1f;
		if (address == 0x10) address = 0x00;
		if (address == 0x14) address = 0x04;
		if (address == 0x18) address = 0x08;
		if (address == 0x1c) address = 0x0c;

		value = pallet[address];

	} else {
		printf("read in address %04x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}

	return value;
}

void ppu_write(uint8_t value, uint16_t address) {

	address &= 0x3fff;
	if (address <= 0x1fff) {
		ppu_mapper_write(value, address);

	} else if (address >= 0x2000 && address <= 0x3eff) {
		address &= 0x2fff;
		if (get_mirroring()) { // vertical mirroring
			vram[address & 0x7ff] = value;

		} else { // horizontal mirroring
			if (address <= 0x23ff) {
				vram[address & 0x3ff] = value;
			} else  if (address <= 0x27ff) {
				vram[(address & 0x3ff) + 0x400] = value;
			} else if (address <= 0x2bff) {
				vram[address & 0x3ff] = value;
			} else if (address <= 0x2fff) {
				vram[(address & 0x3ff) + 0x400] = value;
			} else {
				printf("Something went wrong writing vram at %04x", address);
			}

		}

	} else if (address >= 0x3f00 && address <= 0x3fff) {
		address &= 0x1f;
		if (address == 0x10) address = 0x00;
		if (address == 0x14) address = 0x04;
		if (address == 0x18) address = 0x08;
		if (address == 0x1c) address = 0x0c;

		pallet[address] = value;

	} else {
		printf("write in address %04x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}
}

uint8_t ppu_registers_read(uint16_t address) {

	uint8_t value = 0;
	switch(address & 0x7) {

		case 0x0:
			printf("Reading write only register %02x\n.", address);
			break;

		case 0x1:
			printf("Reading write only register %02x\n.", address);
			break;

		case 0x2:
			value = colapse_ppustatus();
			ppustatus.v_blank = 0;
			t_loopy = 0; // Not sure what is reset
			v_loopy = 0;
			w_loopy = false;
			break;

		case 0x3:
			printf("Reading write only register %02x\n.", address);
			break;

		case 0x4:
			value = oam[oamaddr];
			if (scanline < 240) {
				++oamaddr;
			}
			break;

		case 0x5:
			printf("Reading write only register %02x\n.", address);
			break;

		case 0x6:
			printf("Reading write only register %02x\n.", address);
			break;

		case 0x7:
			;
			static uint8_t buffer;
			if (v_loopy <= 0x3eff) {
				value = buffer;
				buffer = ppu_read(v_loopy);
			} else {
				value = ppu_read(v_loopy);
				buffer = ppu_read((v_loopy & 0x00ff) | 0x2c00);
			}
			v_loopy += ppuctrl.inc ? 32 : 1; 
			break;

	}

	return value;
}

void ppu_registers_write(uint8_t value, uint16_t address) {

	switch(address & 0x7) {

		case 0x0:
			ppuctrl.nt_addr = value;
			ppuctrl.inc = value >> 2;
			ppuctrl.spr_addr = value >> 3;
			ppuctrl.bck_addr = value >> 4;
			ppuctrl.spr_size = value >> 5;
			ppuctrl.master_slave = value >> 6;
			ppuctrl.nmi = value >> 7;

			t_loopy = (t_loopy & 0x7cff) | (ppuctrl.nt_addr << 10);
			ppustatus.last_write = value;
			break;

		case 0x1:
			ppumask.greyscale = value;
			ppumask.bck_left = value >> 1;
			ppumask.spr_left = value >> 2;
			ppumask.background = value >> 3;
			ppumask.sprites = value >> 4;
			ppumask.emph_red = value >> 5;
			ppumask.emph_green = value >> 6;
			ppumask.emph_blue = value >> 7;
			break;

		case 0x2:
			printf("Writing read only register %02x", address);
			break;

		case 0x3:
			oamaddr = value;
			break;

		case 0x4:
			oamaddr = value;
			oam_write(value, oamaddr);
			++oamaddr;
			break;

		case 0x5:
			ppuscroll = value;
			if (!w_loopy) {
				t_loopy = (t_loopy & 0x7fe0) | (value >> 3);
				x_loopy = value & 0x07;
				w_loopy ^= 1;
			} else {
				t_loopy = (t_loopy & 0x831f) | ((value & 0xf8) << 5) | ((value & 0x07) << 12);
				w_loopy ^= 1;
			}
			break;

		case 0x6:
			ppuaddr = value;
			if (!w_loopy) {
				t_loopy = (t_loopy & 0x00ff) | (value << 8);
				t_loopy = t_loopy & 0x7fff;
				w_loopy ^= 1;
			} else {
				t_loopy = (t_loopy & 0xff00) | value;
				v_loopy = t_loopy;
				w_loopy ^= 1;
			}
			break;
		
		case 0x7:
			ppudata = value;
			ppu_write(value, v_loopy);
			if ((scanline <= 239) && (ppumask.background || ppumask.sprites)) {
				v_loopy += 33;

			} else {
				v_loopy += ppuctrl.inc ? 32 : 1;
			}
	}	

}

uint8_t debug_ppu_read(uint16_t address) {

	uint8_t value;

	address &= 0x3fff;
	if (address <= 0x1fff) {
		value = debug_ppu_mapper_read(address);

	} else if (address >= 2000 && address <= 0x3eff) {
		address &= 0x2fff;
		if (get_mirroring()) { // vertical mirroring
			value = vram[address & 0x7ff];

		} else { // horizontal mirroring
			if (address <= 0x23ff) {
				value = vram[address & 0x3ff];
			} else  if (address <= 0x27ff) {
				value = vram[(address & 0x3ff) + 0x400];
			} else if (address <= 0x2bff) {
				value = vram[address & 0x3ff];
			} else if (address <= 0x2fff) {
				value = vram[(address & 0x3ff) + 0x400];
			} else {
				printf("Something went wrong reading vram at %04x", address);
			}

		}

	} else if (address >= 0x3f00 && address <= 0x3fff) {
		address &= 0x1f;
		if (address == 0x10) address = 0x00;
		if (address == 0x14) address = 0x04;
		if (address == 0x18) address = 0x08;
		if (address == 0x1c) address = 0x0c;

		value = pallet[address];

	} else {
		printf("read in address %04x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}

	return value;
}

void oam_write(uint8_t value, uint8_t address) {

	oam[address] = value;
}

uint8_t colapse_ppustatus(void) {

	uint8_t value = ppustatus.last_write | (ppustatus.spr_overflow << 5) | 
					(ppustatus.spr_0hit << 6) | (ppustatus.v_blank << 7);

	return value;
}

size_t get_ppu_cycle(void) {
	return cycle;
}

size_t get_scanline(void) {
	return scanline;  
}

void increment_horizontal(void) {

	if ((v_loopy & 0x001f) == 31) {
					v_loopy &= ~0x001f;
					v_loopy ^= 0x0400;
				} else {
					++v_loopy;
				}
}

void increment_vertical(void) {

	if ((v_loopy & 0x7000) != 0x7000) {
		v_loopy += 0x1000;
	} else {
		v_loopy &= ~0x7000;

		unsigned int corse_y = (v_loopy & 0x03e0) >> 5;
		if (corse_y == 29) {
			corse_y = 0;
			v_loopy ^= 0x0800;
		} else if (corse_y == 31) {
			corse_y = 0;
		} else {
			++corse_y; 
		}
		v_loopy = (v_loopy & ~0x03e0) | (corse_y << 5);
	}
}