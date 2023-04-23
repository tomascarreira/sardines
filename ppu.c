#include "common.h"
#include "ppu.h"
#include "cpu.h"
#include "cartridge.h"
#include "sdl.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

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
static sprite oam[OAM_SPRITE_NUMBER] = { 0 };
static sprite secondary_oam[SECONDARY_OAM_SPRITE_NUMBER] = { 0 };
static size_t sec_oam_len = 0;

static uint8_t spr_tile_data[SECONDARY_OAM_SPRITE_NUMBER][2] = { 0 };
static uint8_t spr_attr_data[SECONDARY_OAM_SPRITE_NUMBER] = { 0 };
static uint8_t spr_x_counter[SECONDARY_OAM_SPRITE_NUMBER] = { 0 };

static size_t frame = 1;
static size_t scanline = 0;
static size_t cycle = 21;

void init_ppu(void) {

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

	// Rendering
	if ((ppumask.background == 1 || ppumask.sprites == 1) && (scanline <= 239 || scanline == 261)) {		

		if ((cycle >= 1 && cycle <= 256) || (cycle >= 321 && cycle <= 336)) {
			
			bg_shift_patt_lo <<= 1;
			bg_shift_patt_hi <<= 1;
			bg_shift_attr_lo <<= 1;
			bg_shift_attr_hi <<= 1;

			if (scanline != 261 && (cycle >= 1 && cycle <= 256)) {
				uint8_t bg_pixel = 0;
				uint8_t spr_pixel = 0;

				uint8_t spr_pallete = 0;
				uint8_t spr_priority = 0;

				if (ppumask.background) {
					uint8_t bg_pixel_lo = (bg_shift_patt_lo >> (15 - x_loopy)) & 0x0001;
					uint8_t bg_pixel_hi = (bg_shift_patt_hi >> (15 - x_loopy)) & 0x0001;
					bg_pixel = (bg_pixel_hi << 1) | bg_pixel_lo;
				}

				if (ppumask.sprites) {
					// Im decrementing x counter after checking for zero
					// because if the value start at zero it will loop to // MAX UINT8 (255) and will not be drawn


					// Problem: Even after the sprite is drawn fully it still get checked
					for (size_t i = 0; i < SECONDARY_OAM_SPRITE_NUMBER; ++i) {
						if (!spr_x_counter[i]) {

							uint8_t top_tile_pix = spr_tile_data[i][0] >> 7;
							uint8_t bot_tile_pix = spr_tile_data[i][1] >> 7;

							// Improvement: Send an sprite type instead
							uint8_t spr_attr = spr_attr_data[i];
							spr_pallete = spr_attr & 0x3;
							spr_priority = (spr_attr >> 5) & 0x1;

							spr_pixel = (bot_tile_pix << 1) + top_tile_pix;

							spr_tile_data[i][0] <<= 1;
							spr_tile_data[i][1] <<= 1;

							if (spr_pixel) {
								break;
							}
							} else {
							--spr_x_counter[i];
						}
					}
				}
				
				uint8_t color;
				// Improvement: Function to get address for colour
				if (!bg_pixel && !spr_pixel) {
					color = ppu_read(0x3f00);

				} else if (!bg_pixel && spr_pixel) {
					uint16_t pallet_addr = spr_pixel + (spr_pallete << 2) + (1 << 4);
					color = ppu_read(pallet_addr + 0x3f00);	

				} else if (bg_pixel && !spr_pixel) {
					uint16_t pallet_addr =((bg_shift_attr_hi >> (15 - x_loopy)) << 3) | ((bg_shift_attr_lo >> (15 - x_loopy)) << 2) | bg_pixel; 	
					color = ppu_read(pallet_addr + 0x3f00);

				} else if (bg_pixel && spr_pixel && !spr_priority) {
					uint16_t pallet_addr = spr_pixel + (spr_pallete << 2) + (1 << 4);
					color = ppu_read(pallet_addr + 0x3f00);	

				} else if (bg_pixel && spr_pixel && spr_priority) {
					uint16_t pallet_addr =((bg_shift_attr_hi >> (15 - x_loopy)) << 3) | ((bg_shift_attr_lo >> (15 - x_loopy)) << 2) | bg_pixel; 	
					color = ppu_read(pallet_addr + 0x3f00);
				} else {
					printf("This should not happend");
					exit(EXIT_FAILURE);
				}

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
	
	} else if (scanline == 241 && cycle == 1) {
		ppustatus.v_blank = 1;
		if (ppuctrl.nmi == 1) {
			nmi();
		}

	} else if (scanline == 261 && cycle == 1) {
		ppustatus.v_blank = 0;
		ppustatus.spr_0hit = 0;
		ppustatus.spr_overflow = 0;
	
	} else if (scanline == 261 && cycle >= 280 && cycle <= 304 && (ppumask.background == 1 || ppumask.sprites == 1)) {
	
		v_loopy = (v_loopy & 0x041f) | (t_loopy & ~0x041f);
	}

	// Sprite Evaluation
	if (scanline >= 0 && scanline <= 239 && (ppumask.sprites || ppumask.background)) {
		// setup the secondary_oam
		if (cycle == 1) {
			memset(secondary_oam, 0xff, SECONDARY_OAM_SPRITE_NUMBER*sizeof(sprite));
			ppustatus.spr_overflow = 0;
			sec_oam_len = 0;
		}

		// Populate the secondary_oam
		if (cycle == 65) {
			for (size_t i = 0; i < OAM_SPRITE_NUMBER; ++i) {
				if (sec_oam_len < 8) {
					if (sprite_in_scanline(scanline, oam[i].top_y_pos)) {
						secondary_oam[sec_oam_len] = oam[i];
						++sec_oam_len;
					}
				} else {
					// Handle sprite overflow flag (that is broken on the real hardware)
					// I will implement the correct way beause i dont understant yet 
					// how m behaves if it overflow >3. m is from the nesdev wiki
					if (sprite_in_scanline(scanline, oam[i].top_y_pos)) {
						ppustatus.spr_overflow = 1;
					}
				}
			}
			draw_secondary_oam();
		}

		// put sprite information ready for next scanline
		if (cycle == 257) {
			for (size_t i = 0; i < SECONDARY_OAM_SPRITE_NUMBER; ++i) {
				if (i >= sec_oam_len) {
					// What to put in the shift registers and latches for the empty slots ??
				} else if (ppuctrl.spr_size) {
					sprite spr = secondary_oam[i];
					uint8_t pt_section = spr.tile_idx & 0x1;
					uint8_t tile_idx = spr.tile_idx >> 1;
					// scanline is always bigger or equal to top_y_pos.
					// Because if top_y_pos was bigger this sprite would
					// never be in secondary_oam
					assert(scanline >= spr.top_y_pos);
					uint8_t y_offset = scanline - spr.top_y_pos;
					if (spr.attributes.flip_v) {
						y_offset = 8 - y_offset;
					}
					assert(y_offset < 16);
					spr_tile_data[i][0] = pattern_table_encode_address(tile_idx, pt_section, y_offset, 0, 16);
					spr_tile_data[i][1] = pattern_table_encode_address(tile_idx, pt_section, y_offset, 1, 16);
					spr_attr_data[i] = spr_attr_to_byte(spr.attributes);
					spr_x_counter[i] = spr.left_x_pos;

					if (spr.attributes.flip_h) {
						spr_tile_data[i][0] = reverse_byte(spr_tile_data[i][0]);
						spr_tile_data[i][1] = reverse_byte(spr_tile_data[i][1]);
					}
				} else {
					sprite spr = secondary_oam[i];
					uint8_t tile_idx = spr.tile_idx;
					// scanline is always bigger or equal to top_y_pos.
					// Because if top_y_pos was bigger this sprite would
					// never be in secondary_oam
					assert(scanline >= spr.top_y_pos);
					uint8_t y_offset = scanline - spr.top_y_pos;
					if (spr.attributes.flip_v) {
						y_offset = 8 - y_offset;
					}
					assert(y_offset < 8);
					spr_tile_data[i][0] = ppu_read(pattern_table_encode_address(tile_idx, ppuctrl.spr_addr, y_offset, 0, 8));
					spr_tile_data[i][1] = ppu_read(pattern_table_encode_address(tile_idx, ppuctrl.spr_addr, y_offset, 1, 8));
					spr_attr_data[i] = spr_attr_to_byte(spr.attributes);
					spr_x_counter[i] = spr.left_x_pos;

					if (spr.attributes.flip_h) {
						spr_tile_data[i][0] = reverse_byte(spr_tile_data[i][0]);
						spr_tile_data[i][1] = reverse_byte(spr_tile_data[i][1]);
					}
				}
			}
		}
	}

	if (scanline == 239 && cycle == 256) {
		present_frame();
	
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
		draw_oam();
	}
}

uint8_t ppu_read(uint16_t address) {

	uint8_t value;

	address &= 0x3fff;
	if (address >= 0x3f00 && address <= 0x3fff) {
		address &= 0x1f;
		if ((address & 0x13) == 0x10) {
			address &= 0x0f;
		}
		value = pallet[address];

	} else if (address <= 0x1fff) {
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

	} else {
		printf("read in address %04x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}

	return value;
}

void ppu_write(uint8_t value, uint16_t address) {

	address &= 0x3fff;
	if (address >= 0x2000 && address <= 0x3eff) {
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

	} else if (address <= 0x1fff) {
		ppu_mapper_write(value, address);

	} else if (address >= 0x3f00 && address <= 0x3fff) {
		address &= 0x1f;
		if ((address & 0x13) == 0x10) {
			address &= 0x0f;
		}
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
			// Read 2004 in cycles 1 to 64 return 0xff
			if (cycle >= 1 && cycle <= 64 && scanline <= 239) {
				value = 0xff;
			} else {
				uint8_t oam_address = oamaddr / 4;
				switch (oamaddr % 3) {
					case 0: value = oam[oam_address].top_y_pos; break;
					case 1: value = oam[oam_address].tile_idx; break;
					case 2: value = spr_attr_to_byte(oam[oam_address].attributes); break;
					case 3: value = oam[oam_address].left_x_pos; break;
				}
			}

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

sprite debug_oam_read(size_t i) {
	return oam[i];
}

sprite debug_sec_oam_read(size_t i) {
	return secondary_oam[i];
}

size_t get_sec_oam_len(void) {
	return sec_oam_len;
}

void oam_write(uint8_t value, uint8_t address) {

	uint8_t oam_address = address / 4;
	
	switch (address % 4) {
		case 0: oam[oam_address].top_y_pos = value; break;
		case 1: oam[oam_address].tile_idx = value; break;
		case 3: oam[oam_address].left_x_pos = value; break;
		case 2: 
			oam[oam_address].attributes.pallet = value;
			oam[oam_address].attributes.unimplemented = 0;
			oam[oam_address].attributes.priority = value >> 5;
			oam[oam_address].attributes.flip_h = value >> 6;
			oam[oam_address].attributes.flip_v = value >> 7;
			break;
	}
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

bool sprite_in_scanline(size_t scanline, uint8_t spr_y_pos) {
	return scanline >= spr_y_pos && scanline < spr_y_pos + (8 + ppuctrl.spr_size * 8);
}

uint8_t spr_attr_to_byte(spr_attr attr) {
	uint8_t res = 0;
	res += attr.pallet;
	res += attr.priority << 5;
	res += attr.flip_h << 6;
	res += attr.flip_v << 7;

	return res;
}

// 0S RRRC CCCP YYYY => maybe a formula for the address in the vram for 8x16 tiles
// S is the section in the pattern table
// R is the row of the tile
// C is the column of the tile
// P is plane inside of the tile (0 for the first, 1 for the second)
// Y is the row inside the tile
uint16_t pattern_table_encode_address(uint8_t tile_idx, uint section, uint y_offset, uint plane, uint spr_size) {
	uint16_t res = 0;

	if (spr_size == 16) {
		res += y_offset;
		res += plane << 4;
		res += (tile_idx & 0xf) << 5;
		res += (tile_idx >> 4) << 9;
		res += section << 12;
	} else {
		res += y_offset;
		res += plane << 3;
		res += (tile_idx & 0xf) << 4;
		res += (tile_idx >> 4) << 8;
		res += section << 12;
	}

	return res;
}

void debug_oam(void) {
	printf("oam:\n");
	for (size_t i = 0; i < OAM_SPRITE_NUMBER; ++i) {
		printf("%02x %02x %02x %02x\n", 
		 oam[i].top_y_pos,
		 oam[i].tile_idx,
		 spr_attr_to_byte(oam[i].attributes),
		 oam[i].left_x_pos);
	}
}

void debug_secondary_oam(void) {
	printf("secondary oam:\n");
	for (size_t i = 0; i < sec_oam_len; ++i) {
		printf("%02x %02x %02x %02x\n", 
		 secondary_oam[i].top_y_pos,
		 secondary_oam[i].tile_idx,
		 spr_attr_to_byte(secondary_oam[i].attributes),
		 secondary_oam[i].left_x_pos);
	}
}

nes_ppuctrl get_ppuctrl(void) {
	return ppuctrl;
}

uint8_t reverse_byte(uint8_t byte) {
	uint8_t res = 0;

	for (size_t i = 0; i < 8; ++i) {
		res += (byte & 0x1) << (7 - i);
		byte >>= 1;
	}

	return res;
}
