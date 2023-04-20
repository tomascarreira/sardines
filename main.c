#include "common.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "sdl.h"

size_t cycles = 7;

int main(int argc, char* argv[argc+1]) {

	uint8_t* rom = read_rom(argv[1]);
	parse_header(rom);
	init_mapper(rom);

	init_cpu();
	init_ram();
	init_ppu();

	init_sdl();

	size_t i = 2;
	bool step_mode = false;
	bool keep_looping = true;
	while (keep_looping) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			switch (event.type) {
				
				case SDL_QUIT:
					keep_looping = false;
					break;

				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == 1) {
						keep_looping = false;
						break;
					} else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
						SDL_DestroyWindow(SDL_GetWindowFromID(event.window.windowID));
					}

				case SDL_KEYDOWN:
					if (event.key.keysym.scancode == SDL_SCANCODE_S && !step_mode) {
						step_mode = true;	
					} else if (event.key.keysym.scancode == SDL_SCANCODE_S && step_mode) {
						if (i >= 2)  {
							clock_cpu();
							i = -1;
							++cycles;
						}
						clock_ppu();
						++i;
					} else if (event.key.keysym.scancode == SDL_SCANCODE_C) {
						step_mode = false;
					}
			}
		}

		if (!step_mode) {
			if (i >= 2)  {
				clock_cpu();
				i = -1;
				++cycles;
			}
			clock_ppu();
			++i;
		}
	}

	free(rom);
	free_mapper();
	free_ram();

	return EXIT_SUCCESS;
}

