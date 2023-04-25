#include "common.h"
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "sdl.h"
#include "input.h"
#include <bits/time.h>
#include <time.h>

void add_ns(struct timespec* time, long int ns) {
	int s = time->tv_sec + ((time->tv_nsec + ns) / 1000000000);
	ns = (time->tv_nsec + ns) % 1000000000;
	time->tv_sec = s;
	time->tv_nsec = ns;
}

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
		struct timespec start = { 0 };
		clock_gettime(CLOCK_REALTIME, &start);
		struct timespec target = { 0 };
		add_ns(&target, 16666666);

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
					break;

				case SDL_KEYDOWN:
					if (event.key.keysym.scancode == SDL_SCANCODE_X && !step_mode) {
						step_mode = true;	
					} else if (event.key.keysym.scancode == SDL_SCANCODE_X && step_mode) {
						if (i >= 2)  {
							clock_cpu();
							i = -1;
							++cycles;
						}
						clock_ppu();
						++i;
					} else if (event.key.keysym.scancode == SDL_SCANCODE_C) {
						step_mode = false;

					} else if (event.key.keysym.scancode == SDL_SCANCODE_J) {
						set_controller_data(1, a, true);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_B) {
						set_controller_data(1, b, true);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_O) {
						set_controller_data(1, contr_select, true);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_P) {
						set_controller_data(1, contr_start, true);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_W) {
						set_controller_data(1, up, true);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
						set_controller_data(1, down, true);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_A) {
						set_controller_data(1, left, true);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
						set_controller_data(1, right, true);
					}
				break;

				case SDL_KEYUP:
					if (event.key.keysym.scancode == SDL_SCANCODE_J) {
						set_controller_data(1, a, false);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_K) {
						set_controller_data(1, b, false);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_P) {
						set_controller_data(1, contr_select, false);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_O) {
						set_controller_data(1, contr_start, false);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_W) {
						set_controller_data(1, up, false);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
						set_controller_data(1, down, false);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_A) {
						set_controller_data(1, left, false);
					} else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
						set_controller_data(1, right, false);
					}
				break;
			}
		}
		bool frame_ready = false;
		if (!step_mode) {
			while (!frame_ready) {
			if (i >= 2)  {
				clock_cpu();
				i = -1;
				++cycles;
			}
			frame_ready = clock_ppu();
			update_input();
			++i;
			}

			present_frame();
		}
		clock_nanosleep(CLOCK_REALTIME, 0, &target, NULL);
	}

	free(rom);
	free_mapper();
	free_ram();

	return EXIT_SUCCESS;
}


