#include "common.h"
#include "cpu.h"
#include "cartridge.h"

const size_t opcode_cycles_table[256] = {
		7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
		2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
		2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
		2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
		2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
		2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
		2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
		2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
		2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
		2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
		2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
		2, 5, 0, 4, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
};

const size_t (*addressing_mode_table[256])(bus*, cpu*, uint16_t*) = {
		imp, izx, imp, izx,  zp,  zp,  zp,  zp, imp, imm, imp, imm, abl, abl, abl, abl,
		rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
		abl, izx, imp, izx,  zp,  zp,  zp,  zp, imp, imm, imp, imm, abl, abl, abl, abl,
		rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
		imp, izx, imp, izx,  zp,  zp,  zp,  zp, imp, imm, imp, imm, abl, abl, abl, abl,
		rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
		imp, izx, imp, izx,  zp,  zp,  zp,  zp, imp, imm, imp, imm, ind, abl, abl, abl,
		rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
		imm, izx, imm, izx,  zp,  zp,  zp,  zp, imp, imm, imp, imm, abl, abl, abl, abl,
		rel, izy, imp, izy, zpx, zpx, zpy, zpy, imp, aby, imp, aby, abx, abx, aby, aby,
		imm, izx, imm, izx,  zp,  zp,  zp,  zp, imp, imm, imp, imm, abl, abl, abl, abl,
		rel, izy, imp, izy, zpx, zpx, zpy, zpy, imp, aby, imp, aby, abx, abx, aby, aby,
		imm, izx, imm, izx,  zp,  zp,  zp,  zp, imp, imm, imp, imm, abl, abl, abl, abl,
		rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx,
		imm, izx, imm, izx,  zp,  zp,  zp,  zp, imp, imm, imp, imm, abl, abl, abl, abl,
		rel, izy, imp, izy, zpx, zpx, zpx, zpx, imp, aby, imp, aby, abx, abx, abx, abx
};

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

	uint16_t address = 0x6969;

	instr_clocks = opcode_cycles_table[opcode];
	instr_clocks += addressing_mode_table[opcode](bus, cpu, &address);
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

size_t imp(bus* bus, cpu* cpu, uint16_t* address) {
	
	return 0;
}

size_t imm(bus* bus, cpu* cpu, uint16_t* address) {
	
	*address = cpu->pc;
	++cpu->pc;

	return 0;
}

size_t zp(bus* bus, cpu* cpu, uint16_t* address) {
	
	*address = cpu_read(bus, cpu->pc);
	++cpu->pc;

	return 0;
}

size_t zpx(bus* bus, cpu* cpu, uint16_t* address) {

	*address = (cpu_read(bus, cpu->pc) + cpu->x) & 0xff;
	++cpu->pc;

	return 0;
}

size_t zpy(bus* bus, cpu* cpu, uint16_t* address) {

	*address = (cpu_read(bus, cpu->pc) + cpu->y) & 0xff;
	++cpu->pc;

	return 0;
}

size_t abl(bus* bus, cpu* cpu, uint16_t* address) {

	uint8_t lo = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t hi = cpu_read(bus, cpu->pc);
	++cpu->pc;

	*address = (hi << 8) | lo;

	return 0;
}

size_t abx(bus* bus, cpu* cpu, uint16_t* address) {

	uint8_t lo = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t hi = cpu_read(bus, cpu->pc);
	++cpu->pc;

	*address = ((hi << 8) | lo) + cpu->x;

	if (lo + cpu->x > 0xff) {
		return 1;
	}

	return 0;
}

size_t aby(bus* bus, cpu* cpu, uint16_t* address) {

	uint8_t lo = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t hi = cpu_read(bus, cpu->pc);
	++cpu->pc;

	*address = ((hi << 8) | lo) + cpu->y;

	if (lo + cpu->y > 0xff) {
		return 1;
	}

	return 0;
}

size_t rel(bus* bus, cpu* cpu, uint16_t* address) {
	
	*address = cpu_read(bus, cpu->pc);
	++cpu->pc;

	return 0;
}

size_t ind(bus* bus, cpu* cpu, uint16_t* address) {
	
	uint8_t lo = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t hi = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint16_t ptr = (hi << 8)	| lo;

	if (lo == 0xff) {
	   *address = (cpu_read(bus, ptr & 0xff00) << 8) | cpu_read(bus, ptr);

	} else {
		*address = (cpu_read(bus, ptr + 1) << 8) | cpu_read(bus, ptr);
	}

	return 0;
}

size_t izx(bus* bus, cpu* cpu, uint16_t* address) {
	
	uint8_t zp_ptr = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t lo = cpu_read(bus, zp_ptr + cpu->x);
	uint8_t hi = cpu_read(bus, zp_ptr + cpu->x + 1);

	*address = (hi << 8) | lo;

	return 0;
}

size_t izy(bus* bus, cpu* cpu, uint16_t* address) {
	
	uint8_t zp_ptr = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t lo = cpu_read(bus, zp_ptr);
	uint8_t hi = cpu_read(bus, zp_ptr + 1);

	*address = ((hi >> 8) | lo) + cpu->y;

	if (lo + cpu->y > 0xff) {
		return 1;
	}

	return 0;
}