#include "common.h"
#include "sdl.h"
#include "cartridge.h"

static SDL_Window* window;
static SDL_Renderer* renderer;

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
								300*SCALE, 256*SCALE, SDL_WINDOW_RESIZABLE);
	if (!window) {
		printf("SDL_CreaterWindow failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		printf("SDL_CreateRenderer failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

void draw_pattern_table(void) {

	SDL_Window* pt_window;
	SDL_Renderer* pt_renderer;

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

	SDL_SetRenderDrawColor(pt_renderer, 0, 0, 0, 255);
	SDL_RenderClear(pt_renderer);

	uint8_t lo;
	uint8_t hi;

	for (size_t y = 0; y < 128; ++y) {
		for (size_t x = 0; x < 128; ++x) {
			if (!(x % 8)) {
				lo = debug_ppu_mapper_read(y | ((x / 8) << 4) | ((y / 8) << 8));
				hi = debug_ppu_mapper_read(y | (1 << 3) | ((x / 8) << 4) | ((y / 8) << 8));
			}
			
			uint8_t index = ((lo & 0x80) >> 7) | ((hi & 0x80) >> 6);
			lo <<= 1;
			hi <<= 1;
			
			uint8_t color;
			switch (index) {
				case 0: color = 0x1a; break;
				case 1: color = 0x21; break;
				case 2: color = 0x06; break;
				case 3: color = 0x30; break;
			}

			SDL_Rect pixel = {x*SCALE, y*SCALE, SCALE, SCALE};
			SDL_SetRenderDrawColor(pt_renderer, pallet[color].red, pallet[color].green, pallet[color].blue, 255);
			SDL_RenderDrawRect(pt_renderer, &pixel);
			SDL_RenderFillRect(pt_renderer, &pixel);
			
		}
	}

	for (size_t y = 0; y < 128; ++y) {
		for (size_t x = 0; x < 128; ++x) {
			if (!(x % 8)) {
				lo = debug_ppu_mapper_read(y | ((x / 8) << 4) | ((y / 8) << 8));
				hi = debug_ppu_mapper_read(y | (1 << 3) | ((x / 8) << 4) | ((y / 8) << 8) | (1 << 12));
			}
			
			uint8_t index = ((lo & 0x80) >> 7) | ((hi & 0x80) >> 6);
			lo <<= 1;
			hi <<= 1;
			
			uint8_t color;
			switch (index) {
				case 0: color = 0x1a; break;
				case 1: color = 0x21; break;
				case 2: color = 0x06; break;
				case 3: color = 0x30; break;
			}

			SDL_Rect pixel = {(x+128)*SCALE, y*SCALE, SCALE, SCALE};
			SDL_SetRenderDrawColor(pt_renderer, pallet[color].red, pallet[color].green, pallet[color].blue, 255);
			SDL_RenderDrawRect(pt_renderer, &pixel);
			SDL_RenderFillRect(pt_renderer, &pixel);
			
		}
	}

	SDL_RenderPresent(pt_renderer);
}

void draw_pallets(void) {

}

void draw_pixel(size_t x, size_t y, uint8_t color) {

	SDL_Rect pixel = {x, y, SCALE, SCALE};
	SDL_SetRenderDrawColor(renderer, pallet[color].red, pallet[color].green, pallet[color].blue, 255);
	SDL_RenderDrawRect(renderer, &pixel);
	SDL_RenderFillRect(renderer, &pixel);
}