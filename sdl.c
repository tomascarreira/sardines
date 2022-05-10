#include "common.h"
#include "sdl.h"

static SDL_Window* window;
static SDL_Renderer* renderer;

void init_sdl(void) {

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("SLD_Init failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	atexit(SDL_Quit);

	window = SDL_CreateWindow("SardiNES", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								256*SCALE, 240*SCALE, 0);
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