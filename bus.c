#include "common.h"
#include "bus.h"

bus* init_bus(mapper mapper) {
	
	bus* bus = calloc(1, sizeof bus);
	bus->ram = calloc(1, 0x800);
	bus->mapper = mapper;

	return bus;
}