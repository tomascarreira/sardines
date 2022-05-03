#ifndef CPU_H
#define CPU_H

#define STACK_POINTER 0xfd

typedef struct cpu cpu;
struct cpu {
	uint8_t a;
	uint8_t x;
	uint8_t y;
	uint16_t pc;
	uint8_t s;
	struct status {
		uint8_t c:1;
		uint8_t z:1;
		uint8_t i:1;
		uint8_t d:1;
		uint8_t b:2;
		uint8_t o:1;
		uint8_t n:1;
	} p;
};

cpu* init_cpu(bus* bus);
void clock_cpu(cpu* cpu, bus* bus);
uint8_t cpu_read(bus* bus, uint16_t address);
void cpu_write(bus* bus, uint8_t value, uint16_t address);

size_t imp(bus* bus, cpu* cpu, uint16_t* address);
size_t imm(bus* bus, cpu* cpu, uint16_t* address);
size_t zp(bus* bus, cpu* cpu, uint16_t* address);
size_t zpx(bus* bus, cpu* cpu, uint16_t* address);
size_t zpy(bus* bus, cpu* cpu, uint16_t* address);
size_t abl(bus* bus, cpu* cpu, uint16_t* address);
size_t abx(bus* bus, cpu* cpu, uint16_t* address);
size_t aby(bus* bus, cpu* cpu, uint16_t* address);
size_t rel(bus* bus, cpu* cpu, uint16_t* address);
size_t ind(bus* bus, cpu* cpu, uint16_t* address);
size_t izx(bus* bus, cpu* cpu, uint16_t* address);
size_t izy(bus* bus, cpu* cpu, uint16_t* address);

#endif