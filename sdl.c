#include "common.h"
#include "sdl.h"
#include "ppu.h"
#include "cartridge.h"

static SDL_Window* window;
static SDL_Renderer* renderer;

static SDL_Window* pal_window;
static SDL_Renderer* pal_renderer;

static SDL_Window* pt_window;
static SDL_Renderer* pt_renderer;

static SDL_Window* oam_window;
static SDL_Renderer* oam_renderer;

static SDL_Window* sec_oam_window;
static SDL_Renderer* sec_oam_renderer;

static SDL_Texture* texture;

static uint8_t* frame_buffer;

nes_color pallet[64] = {
	{84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136}, {68, 0, 100}, {92, 0, 48}, {84, 4, 0}, {60, 24, 0},
	{0, 32, 42}, {0, 8, 58}, {0, 64, 0}, {0, 60, 0}, {0, 50, 60}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
	{152, 150, 152}, {8,76, 196}, {48, 50, 236}, {92, 30, 228}, {136, 29, 176}, {160, 20, 100}, {152, 34, 32}, {120, 60, 0},
	{84, 90, 0}, {40, 114, 0}, {8, 124, 0}, {0, 118, 40}, {0, 102, 120}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
	{236, 238, 236}, {76, 154, 236}, {120, 124, 236}, {176, 98, 236}, {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32},
	{160, 170, 0}, {116, 196, 0}, {76, 208, 32}, {56, 204, 108}, {56, 180, 204}, {60, 60, 60}, {0, 0, 0}, {0, 0, 0},
	{236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
	{205, 210, 120}, {180, 222, 120}, {168, 226, 144},  {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {0, 0, 0}, {0, 0, 0}
};

void init_sdl(void) {

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SLD_Init failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	atexit(SDL_Quit);

	window = SDL_CreateWindow("SardiNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								256, 240, SDL_WINDOW_RESIZABLE);
	if (!window) {
		printf("SDL_CreaterWindow failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		printf("SDL_CreateRenderer failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	// Improvement: Use only one window for the debug stuff
	pt_window = SDL_CreateWindow("Pattern Tables", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								256*SCALE, 128*SCALE, 0);
	if (!pt_window) {
		printf("SDL_CreaterWindow failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	pt_renderer = SDL_CreateRenderer(pt_window, -1, 0);
	if (!pt_renderer) {
		printf("SDL_CreateRenderer failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	pal_window = SDL_CreateWindow("Pallets", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								8*DEBUG_SCALE + 2, 4*DEBUG_SCALE, 0);
	if (!pal_window) {
		printf("SDL_CreaterWindow failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	pal_renderer = SDL_CreateRenderer(pal_window, -1, 0);
	if (!pal_renderer) {
		printf("SDL_CreateRenderer failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	// Fix: Doesnt work with 8x16 sprites
	oam_window = SDL_CreateWindow("Oam", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
							   OAM_SPRITE_NUMBER*8*DEBUG_SCALE, 1*8*DEBUG_SCALE, 0);
	if (!oam_window) {
		printf("SDL_CreateWindow failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	oam_renderer = SDL_CreateRenderer(oam_window, -1, 0);
	if (!oam_renderer) {
		printf("SDL_CreateRenderer failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	sec_oam_window = SDL_CreateWindow("Secondary oam", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							   SECONDARY_OAM_SPRITE_NUMBER*8*DEBUG_SCALE, 1*8*DEBUG_SCALE, 0);
	if (!sec_oam_window) {
		printf("SDL_CreateWindow failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	sec_oam_renderer = SDL_CreateRenderer(sec_oam_window, -1, 0);
	if (!sec_oam_renderer) {
		printf("SDL_CreateRenderer failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(renderer, 256, 240);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 255, 240);
	if (!texture) {
		printf("SDL_CreateTexture failed: %s\n", SDL_GetError());
	}

	frame_buffer = calloc(3*256*240, 1);
	if (!frame_buffer) {
		printf("Calloc failed\n");
		exit(EXIT_FAILURE);
	}
}

void draw_pattern_table(void) {

	SDL_SetRenderDrawColor(pt_renderer, 0, 0, 0, 255);
	SDL_RenderClear(pt_renderer);

	uint8_t lo = 0;
	uint8_t hi = 0;

	for (size_t y = 0; y < 128; ++y) {
		for (size_t x = 0; x < 128; ++x) {
			if (!(x % 8)) {
				lo = debug_ppu_read((y % 8) | ((x / 8) << 4) | ((y / 8) << 8));
				hi = debug_ppu_read((y % 8) | (1 << 3) | ((x / 8) << 4) | ((y / 8) << 8));
			}
			
			uint8_t color = ((lo & 0x80) >> 7) | ((hi & 0x80) >> 6);
			lo <<= 1;
			hi <<= 1;
			
			uint8_t index = debug_ppu_read(0x3f00 + color);

			SDL_Rect pixel = {x*SCALE, y*SCALE, SCALE, SCALE};
			SDL_SetRenderDrawColor(pt_renderer, pallet[index].red, pallet[index].green, pallet[index].blue, 255);
			SDL_RenderDrawRect(pt_renderer, &pixel);
			SDL_RenderFillRect(pt_renderer, &pixel);
			
		}
	}

	for (size_t y = 0; y < 128; ++y) {
		for (size_t x = 0; x < 128; ++x) {
			if (!(x % 8)) {
				lo = debug_ppu_mapper_read((y % 8) | ((x / 8) << 4) | ((y / 8) << 8) | (1 << 12));
				hi = debug_ppu_mapper_read((y % 8) | (1 << 3) | ((x / 8) << 4) | ((y / 8) << 8) | (1 << 12));
			}
			
			uint8_t color = ((lo & 0x80) >> 7) | ((hi & 0x80) >> 6);
			lo <<= 1;
			hi <<= 1;
			
			uint8_t index = debug_ppu_read(0x3f00 + color);

			SDL_Rect pixel = {(x+128)*SCALE, y*SCALE, SCALE, SCALE};
			SDL_SetRenderDrawColor(pt_renderer, pallet[index].red, pallet[index].green, pallet[index].blue, 255);
			SDL_RenderDrawRect(pt_renderer, &pixel);
			SDL_RenderFillRect(pt_renderer, &pixel);
			
		}
	}

	SDL_RenderPresent(pt_renderer);
}

void draw_pallets(void) {

	SDL_SetRenderDrawColor(pal_renderer, 0, 0, 0, 255);
	SDL_RenderClear(pal_renderer);

	for (size_t y = 0; y < 4; ++y) {
		for (size_t x = 0; x < 4; ++x) {
			uint8_t index = debug_ppu_read(0x3f00 + x + (y*4));
			
			SDL_Rect pixel = {x*DEBUG_SCALE, y*DEBUG_SCALE, DEBUG_SCALE, DEBUG_SCALE};
			nes_color color = pallet[index];
			SDL_SetRenderDrawColor(pal_renderer, color.red, color.green, color.blue, 255);
			SDL_RenderDrawRect(pal_renderer, &pixel);
			SDL_RenderFillRect(pal_renderer, &pixel);
		}
	}

	for (size_t y = 0; y < 4; ++y) {
		for (size_t x = 0; x < 4; ++x) {
			uint8_t index = debug_ppu_read(0x3f10 + x + (y*4));
			
			SDL_Rect pixel = {x*DEBUG_SCALE + DEBUG_SCALE*4 + 2, y*DEBUG_SCALE, DEBUG_SCALE, DEBUG_SCALE};
			nes_color color = pallet[index];
			SDL_SetRenderDrawColor(pal_renderer, color.red, color.green, color.blue, 255);
			SDL_RenderDrawRect(pal_renderer, &pixel);
			SDL_RenderFillRect(pal_renderer, &pixel);
		}
	}

	SDL_RenderPresent(pal_renderer);

}

// Improvement: Support 8x16 sprites
void draw_oam(void) {
	SDL_SetRenderDrawColor(oam_renderer, 0, 0, 0, 255);
	SDL_RenderClear(oam_renderer);

	if (get_ppuctrl().spr_size) {
		printf("Debug oam draw not support for 8x16 sprites.");
	}

	for (size_t i = 0; i < OAM_SPRITE_NUMBER; ++i) {
		sprite spr = debug_oam_read(i); 
		for (size_t j = 0; j < 8; ++j) {
			uint8_t pixel_low = debug_ppu_read(pattern_table_encode_address(spr.tile_idx, get_ppuctrl().spr_addr, j, 0, 8)); 
			uint8_t pixel_high = debug_ppu_read(pattern_table_encode_address(spr.tile_idx, get_ppuctrl().spr_addr, j, 1, 8)); 

			for (size_t k = 0; k < 8; ++k) {
				uint8_t pixel = ((pixel_high >> 7) << 1) + (pixel_low >> 7);
				uint8_t color = debug_ppu_read(0x3f00 + pixel + (spr.attributes.pallet << 2) + (1 << 4));

				SDL_Rect rect = {
					i*8*DEBUG_SCALE + k*DEBUG_SCALE,
					j*DEBUG_SCALE,
					DEBUG_SCALE,
					DEBUG_SCALE
				};

				SDL_SetRenderDrawColor(oam_renderer, pallet[color].red, pallet[color].green, pallet[color].blue, 255);
				SDL_RenderDrawRect(oam_renderer, &rect);
				SDL_RenderFillRect(oam_renderer, &rect);

				pixel_low <<= 1;
				pixel_high <<= 1;
			}
		}
	}
	SDL_RenderPresent(oam_renderer);
}

void draw_secondary_oam(void) {
	SDL_SetRenderDrawColor(sec_oam_renderer, 0, 0, 0, 255);
	SDL_RenderClear(sec_oam_renderer);

	if (get_ppuctrl().spr_size) {
		printf("Debug secondary oam draw not support for 8x16 sprites.");
	}

	for (size_t i = 0; i < get_sec_oam_len(); ++i) {
		sprite spr = debug_sec_oam_read(i); 
		for (size_t j = 0; j < 8; ++j) {
			uint8_t pixel_low = debug_ppu_read(pattern_table_encode_address(spr.tile_idx, get_ppuctrl().spr_addr, j, 0, 8)); 
			uint8_t pixel_high = debug_ppu_read(pattern_table_encode_address(spr.tile_idx, get_ppuctrl().spr_addr, j, 1, 8)); 

			for (size_t k = 0; k < 8; ++k) {
				uint8_t pixel = ((pixel_high >> 7) << 1) + (pixel_low >> 7);
				uint8_t color = debug_ppu_read(0x3f00 + pixel + (spr.attributes.pallet << 2) + (1 << 4));

				SDL_Rect rect = {
					i*8*DEBUG_SCALE + k*DEBUG_SCALE,
					j*DEBUG_SCALE,
					DEBUG_SCALE,
					DEBUG_SCALE
				};

				SDL_SetRenderDrawColor(sec_oam_renderer, pallet[color].red, pallet[color].green, pallet[color].blue, 255);
				SDL_RenderDrawRect(sec_oam_renderer, &rect);
				SDL_RenderFillRect(sec_oam_renderer, &rect);

				pixel_low <<= 1;
				pixel_high <<= 1;
			}
		}
	}
	SDL_RenderPresent(sec_oam_renderer);
}

void draw_pixel(size_t x, size_t y, uint8_t color) {
	frame_buffer[x*3 + y*255*3] = pallet[color].red;
	frame_buffer[x*3 + y*255*3 + 1] = pallet[color].green;
	frame_buffer[x*3 + y*255*3 + 2] = pallet[color].blue;
}

void clear_screen(void) {

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

void present_frame(void) {
	if (SDL_UpdateTexture(texture, NULL, frame_buffer, 255*3)) {
		printf("SDL_UpdateTexture failed: %s\n", SDL_GetError());
	}
	if (SDL_RenderCopy(renderer, texture, NULL, NULL)) {
		printf("SDL_RenderCopy failed: %s\n", SDL_GetError());
	}
	SDL_RenderPresent(renderer);
}
