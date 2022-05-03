#include "common.h"
#include "cpu.h"
#include "cartridge.h"

cpu* init_cpu(bus* bus) {
	
	cpu* cpu = calloc(1, sizeof cpu);
	if (!cpu) {
		perror("cpu calloc failed.\n");
		exit(EXIT_FAILURE);
	}

	cpu->s = STACK_POINTER;
	cpu->p.i = 1; // interrupt ios disable
	cpu->p.b = 3;
	cpu->pc = get_reset_vector(bus);

	return cpu;
}