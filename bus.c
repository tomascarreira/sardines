#include "common.h"
#include "bus.h"

bus* init_bus(mapper mapper) {
	
	bus* bus = calloc(1, sizeof bus);
	if (!bus) {
		perror("bus calloc failed.\n");
		exit(EXIT_FAILURE);
	}

	bus->ram = calloc(1, 0x800);
	if (!ram) {
		perror("ram calloc failed.\n");
		exit(EXIT_FAILURE);
	}

	bus->mapper = mapper;

	return bus;
}