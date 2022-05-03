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

void clock_cpu(cpu* cpu, bus* bus) {
	
	static size_t instr_clocks;

	if (instr_clocks) {
		--instr_clocks;
		return;
	}
	
	uint8_t opcode = cpu_read(bus, cpu->pc);
	++cpu->pc;

}

uint8_t cpu_read(bus* bus,  uint16_t address) {

	if (address <= 0x1fff) {
		return bus->ram[address & 0x7ff];

	} else if (address >= 0x4020 && address <= 0xffff) {
		return mapper_read(bus, address);
	
	} else {
		printf("Read bus address %02x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}
}

void cpu_write(bus* bus, uint8_t value, uint16_t address) {
	
	if (address <= 0x1ff) {
		bus->ram[address & 0xff] = value;

	} else if (address >= 0x4020 && address <= 0xffff) {
		mapper_write(bus, value, address);

	} else {
		printf("Write bus address %02x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}
}