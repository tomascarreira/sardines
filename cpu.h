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
uint8_t cpu_read(bus* bus, uint16_t address);
void cpu_write(bus* bus, uint8_t value, uint16_t address);

#endif