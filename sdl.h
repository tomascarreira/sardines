#ifndef SDL_H
#define SDL_H

#include <stdlib.h>
#include <stdint.h>

#define SCALE 3
#define DEBUG_SCALE 2
#define RENDER_SCALE 4.0

typedef struct nes_color nes_color;
struct nes_color{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

void init_sdl(void);
void draw_pattern_table(void);
void draw_pallets(void);
void draw_oam(void);
void draw_secondary_oam(void);
void draw_pixel(size_t x, size_t y, uint8_t color);
void clear_screen(void);
void present_frame(void);

#endif

