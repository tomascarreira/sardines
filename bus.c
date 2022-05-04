#include "common.h"
#include "bus.h"

nes_bus* init_bus(nes_mapper mapper) {
	
	nes_bus* bus = calloc(1, sizeof (nes_bus));
	if (!bus) {
		perror("bus calloc failed.\n");
		exit(EXIT_FAILURE);
	}

	uint8_t* ram = calloc(0x800, 1);
	if (!ram) {
		perror("ram calloc failed.\n");
		exit(EXIT_FAILURE);
	}

	bus->ram = ram;
	bus->mapper = mapper;

	return bus;
}