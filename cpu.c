#include "common.h"
#include "cpu.h"
#include "cartridge.h"
#include "log.h"

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

const size_t (*addressing_mode_table[256])(nes_bus*, nes_cpu*, uint16_t*) = {
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

const size_t (*opcode_table[256])(nes_bus*, nes_cpu*, uint16_t) = {
		brk, ora, kil, slo, nop, ora, asl, slo, php, ora, asl_a, anc, nop, ora, asl, slo,
		bpl, ora, kil, slo, nop, ora, asl, slo, clc, ora, nop, slo, nop, ora, asl, slo,
		jsr, and, kil, rla, bit, and, rol, rla, plp, and, rol_a, anc, bit, and, rol, rla,
		bmi, and, kil, rla, nop, and, rol, rla, sec, and, nop, rla, nop, and, rol, rla,
		rti, eor, kil, sre, nop, eor, lsr, sre, pha, eor, lsr_a, alr, jmp, eor, lsr, sre,
		bvc, eor, kil, sre, nop, eor, lsr, sre, cli, eor, nop, sre, nop, eor, lsr, sre,
		rts, adc, kil, rra, nop, adc, ror, rra, pla, adc, ror_a, arr, jmp, adc, ror, rra,
		bvs, adc, kil, rra, nop, adc, ror, rra, sei, adc, nop, rra, nop, adc, ror, rra,
		nop, sta, nop, sax, sty, sta, stx, sax, dey, nop, txa, xaa, sty, sta, stx, sax,
		bcc, sta, kil, ahx, sty, sta, stx, sax, tya, sta, txs, tas, shy, sta, shx, ahx,
		ldy, lda, ldx, lax, ldy, lda, ldx, lax, tay, lda, tax, lax, ldy, lda, ldx, lax,
		bcs, lda, kil, lax, ldy, lda, ldx, lax, clv, lda, tsx, las, ldy, lda, ldx, lax,
		cpy, cmp, nop, dcp, cpy, cmp, dec, dcp, iny, cmp, dex, axs, cpy, cmp, dec, dcp,
		bne, cmp, kil, dcp, nop, cmp, dec, dcp, cld, cmp, nop, dcp, nop, cmp, dec, dcp,
		cpx, sbc, nop, isc, cpx, sbc, inc, isc, inx, sbc, nop, sbc, cpx, sbc, inc, isc,
		beq, sbc, kil, isc, nop, sbc, inc, isc, sed, sbc, nop, isc, nop, sbc, inc, isc
};

nes_cpu* init_cpu(nes_bus* bus) {
	
	nes_cpu* cpu = calloc(1, sizeof cpu);
	if (!cpu) {
		perror("cpu calloc failed.\n");
		exit(EXIT_FAILURE);
	}

	cpu->s = STACK_POINTER;
	cpu->p.i = 1;
	cpu->p.b = 3;
	cpu->pc = get_reset_vector(bus);

	return cpu;
}

void clock_cpu(nes_cpu* cpu, nes_bus* bus) {
	
	static size_t instr_clocks;

	if (instr_clocks) {
		--instr_clocks;
		return;
	}
	
	log_instr(bus, cpu);

	uint8_t opcode = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint16_t address;

	instr_clocks = opcode_cycles_table[opcode];
	instr_clocks += addressing_mode_table[opcode](bus, cpu, &address);
	instr_clocks += opcode_table[opcode](bus, cpu, address);
	
	--instr_clocks;
}

uint8_t cpu_read(nes_bus* bus,  uint16_t address) {

	if (address <= 0x1fff) {
		return bus->ram[address & 0x7ff];

	} else if (address >= 0x2000 && address <= 0x3fff) {
		return 0;

	} else if (address >= 0x4000 && address <= 0x401f){
		return 0;

	} else if (address >= 0x4020 && address <= 0xffff) {
		return mapper_read(bus, address);
	
	} else {
		printf("Read bus address %02x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}
}

void cpu_write(nes_bus* bus, uint8_t value, uint16_t address) {
	
	if (address <= 0x7ff) {
		bus->ram[address & 0x7ff] = value;

	} else if (address >= 0x2000 && address <= 0x3fff) {


	} else if (address >= 0x4000 && address <= 0x401f){


	} else if (address >= 0x4020 && address <= 0xffff) {
		mapper_write(bus, value, address);

	} else {
		printf("Write bus address %02x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}
}

size_t imp(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {
	
	return 0;
}

size_t imm(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {
	
	*address = cpu->pc;
	++cpu->pc;

	return 0;
}

size_t zp(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {
	
	*address = cpu_read(bus, cpu->pc);
	++cpu->pc;

	return 0;
}

size_t zpx(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {

	uint8_t operand =	cpu_read(bus, cpu->pc);
	++cpu->pc;

	*address = (operand + cpu->x) & 0xff;

	return 0;
}

size_t zpy(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {

	uint8_t operand =	cpu_read(bus, cpu->pc);
	++cpu->pc;

	*address = (operand + cpu->y) & 0xff;

	return 0;
}

size_t abl(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {

	uint8_t lo = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t hi = cpu_read(bus, cpu->pc);
	++cpu->pc;

	*address = (hi << 8) | lo;

	return 0;
}

size_t abx(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {

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

size_t aby(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {

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

size_t rel(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {
	
	*address = cpu_read(bus, cpu->pc);
	++cpu->pc;

	return 0;
}

size_t ind(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {
	
	uint8_t lo = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t hi = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint16_t ptr = (hi << 8) | lo;

	if (lo == 0xff) {
	   *address = (cpu_read(bus, ptr & 0xff00) << 8) | cpu_read(bus, ptr);

	} else {
		*address = (cpu_read(bus, ptr + 1) << 8) | cpu_read(bus, ptr);
	}

	return 0;
}

size_t izx(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {
	
	uint8_t zp_ptr = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t lo = cpu_read(bus, (zp_ptr + cpu->x) & 0xff);
	uint8_t hi = cpu_read(bus, (zp_ptr + cpu->x + 1) & 0xff);

	*address = (hi << 8) | lo;

	return 0;
}

size_t izy(nes_bus* bus, nes_cpu* cpu, uint16_t* address) {
	
	uint8_t zp_ptr = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint8_t lo = cpu_read(bus, zp_ptr);
	uint8_t hi = cpu_read(bus, (zp_ptr + 1) & 0xff);

	*address = ((hi << 8) | lo) + cpu->y;

	if (lo + cpu->y > 0xff) {
		return 1;
	}

	return 0;
}

size_t adc(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint16_t result = operand + cpu->a + cpu->p.c;

	cpu->p.c = result > 0xff;
	cpu->p.z = (uint8_t) result == 0x00;
	cpu->p.v = (((uint8_t) result ^ cpu->a) & ((uint8_t) result ^ operand)) >> 7;
	cpu->p.n = (int8_t) result < 0;

	cpu->a = result;
	
	return 0;
}

size_t and(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->a &= cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t asl(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint8_t result = operand << 1;

	cpu_write(bus, result, address);

	cpu->p.c = operand >> 7;
	cpu->p.z = result == 0;
	cpu->p.n = (int8_t) result < 0;

	return 0;
}

size_t asl_a(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	uint8_t operand = cpu->a;

	cpu->a <<= 1;

	cpu->p.c = operand >> 7;
	cpu->p.z = cpu->a == 0;
	cpu->p.n = (int8_t) cpu->a < 0;

	return 0;
}

size_t bcc(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (!cpu->p.c) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0; 
}

size_t bcs(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (cpu->p.c) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t beq(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (cpu->p.z) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t bit(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);

	uint8_t tmp = operand & cpu->a;

	cpu->p.z = tmp == 0;
	cpu->p.v = (operand >> 6) & 0x01;
	cpu->p.n = operand >> 7;

	return 0;
}
size_t bmi(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (cpu->p.n) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t bne(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (!cpu->p.z) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t bpl(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (!cpu->p.n) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t brk(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	++cpu->pc;
	
	push(bus, cpu, cpu->pc >> 8);
	push(bus, cpu, cpu->pc);

	cpu->p.b = 3;
	push(bus, cpu, colapse_status(cpu));

	cpu->pc = get_irq_vector(bus);
	
	return 0;
}
size_t bvc(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (!cpu->p.v) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t bvs(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (cpu->p.v) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t clc(nes_bus* bus, nes_cpu* cpu, uint16_t address) { 

	cpu->p.c = 0;

	return 0;
}
size_t cld(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	cpu->p.d = 0;

	return 0;
}
size_t cli(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	cpu->p.i = 0;

	return 0;
}
size_t clv(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	cpu->p.v = 0;

	return 0;
}
size_t cmp(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);

	cpu->p.c = cpu->a >= operand;
	cpu->p.z = cpu->a == operand;
	cpu->p.n = (cpu->a - operand) >> 7; 

	return 0;
}
size_t cpx(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);

	cpu->p.c = cpu->x >= operand;
	cpu->p.z = cpu->x == operand;
	cpu->p.n = (cpu->x - operand) >> 7;
	
	return 0;
}

size_t cpy(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);

	cpu->p.c = cpu->y >= operand;
	cpu->p.z = cpu->y == operand;
	cpu->p.n = (cpu->y - operand) >> 7; 
	
	return 0;
}
size_t dec(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	--operand;

	cpu_write(bus, operand, address);

	cpu->p.z = operand == 0;
	cpu->p.n = operand >> 7;
	
	return 0;
}
size_t dex(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	--cpu->x;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}
size_t dey(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	--cpu->y;

	cpu->p.z = cpu->y == 0;
	cpu->p.n = cpu->y >> 7;
	
	return 0;
}
size_t eor(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->a ^= cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t inc(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);
	++operand;

	cpu_write(bus, operand, address);

	cpu->p.z = operand == 0;
	cpu->p.n = operand >> 7;
	
	return 0;

	return 0;
}
size_t inx(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	++cpu->x;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t iny(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	++cpu->y;

	cpu->p.z = cpu->y == 0;
	cpu->p.n = cpu->y >> 7;
	
	return 0;
}

size_t jmp(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->pc = address;
	
	return 0;
}

size_t jsr(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	push(bus, cpu, (cpu->pc -1) >> 8);
	push(bus, cpu, cpu->pc - 1);

	cpu->pc = address;
	
	return 0;
}

size_t lda(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->a = cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t ldx(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->x = cpu_read(bus, address);

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t ldy(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->y = cpu_read(bus, address);

	cpu->p.z = cpu->y == 0;
	cpu->p.n = cpu->y >> 7;
	
	return 0;
}

size_t lsr(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint8_t result = operand >> 1;

	cpu_write(bus, result, address);

	cpu->p.c = operand;
	cpu->p.z = result == 0;
	cpu->p.n = 	result >> 7;
	
	return 0;
}

size_t lsr_a(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu->a;

	cpu->a = operand >> 1;

	cpu->p.c = operand;
	cpu->p.z = cpu->a == 0;
	cpu->p.n = 	cpu->a >> 7;
	
	return 0;
}

size_t nop(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	return 0;
}

size_t ora(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->a |= cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t pha(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	push(bus, cpu, cpu->a);
	
	return 0;
}

size_t php(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t b_flag = cpu->p.b;

	cpu->p.b = 3;	
	push(bus, cpu, colapse_status(cpu));
	cpu->p.b = b_flag;
	
	return 0;
}

size_t pla(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->a = pop(bus, cpu);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t plp(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t status = pop(bus, cpu);
	cpu->p.c = status;
	cpu->p.z = status >> 1;
	cpu->p.i = status >> 2;
	cpu->p.d = status >> 3;
	cpu->p.v = status >> 6;
	cpu->p.n = status >> 7;
	
	return 0;
}

size_t rol(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint8_t result = (operand << 1) | cpu->p.c;

	cpu_write(bus, result, address);

	cpu->p.c = operand >> 7;
	cpu->p.z = result == 0;
	cpu->p.n = result >> 7;
	
	return 0;
}

size_t rol_a(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	uint8_t operand = cpu->a;

	cpu->a = (cpu->a << 1) | cpu->p.c;

	cpu->p.c = operand >> 7;
	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;	 

	return 0;
}

size_t ror(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint8_t result = (operand >> 1) | (cpu->p.c << 7);

	cpu_write(bus, result, address);

	cpu->p.c = operand;
	cpu->p.z = result == 0;
	cpu->p.n = result >> 7;
	
	return 0;
}

size_t ror_a(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu->a;

	cpu->a = (cpu->a >> 1) | (cpu->p.c << 7);

	cpu->p.c = operand;
	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;	
	
	return 0;
}

size_t rti(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t status = pop(bus, cpu);
	cpu->p.c = status;
	cpu->p.z = status >> 1;
	cpu->p.i = status >> 2;
	cpu->p.d = status >> 3;
	cpu->p.v = status >> 6;
	cpu->p.n = status >> 7;

	uint8_t lo = pop(bus, cpu);
	uint8_t hi = pop(bus, cpu);

	cpu->pc = (hi << 8) | lo;
	
	return 0;
}

size_t rts(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t lo = pop(bus, cpu);
	uint8_t hi = pop(bus, cpu);

	cpu->pc = (hi << 8) | lo;
	++cpu->pc;
	
	return 0;
}

size_t sbc(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = ~cpu_read(bus, address);
	uint16_t result = operand + cpu->a + cpu->p.c;

	cpu->p.c = result > 0xff;
	cpu->p.z = (uint8_t) result == 0x00;
	cpu->p.v = (((uint8_t) result ^ cpu->a) & ((uint8_t) result ^ operand)) >> 7;
	cpu->p.n = (int8_t) result < 0; 

	cpu->a = result;
	
	return 0; 
}

size_t sec(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->p.c = 1;
	
	return 0;
}

size_t sed(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->p.d = 1;
	
	return 0;
}

size_t sei(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->p.i = 1;
	
	return 0;
}

size_t sta(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu_write(bus, cpu->a, address);
	
	return 0;
}

size_t stx(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu_write(bus, cpu->x, address);
	
	return 0;
}

size_t sty(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu_write(bus, cpu->y, address);

	return 0;
}

size_t tax(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->x = cpu->a;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t tay(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->y = cpu->a;

	cpu->p.z = cpu->y == 0;
	cpu->p.n = cpu->y >> 7;
	
	return 0;
}

size_t tsx(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->x = cpu->s;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t txa(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->a = cpu->x;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t txs(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->s = cpu->x;

	return 0;
}

size_t tya(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu->a = cpu->y;

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t ahx(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	uint8_t hi_before = (cpu->pc - 1) >> 8;

	cpu_write(bus, cpu->a & cpu->x & (hi_before + 1), address);

	return 0;
}

size_t alr(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	and(bus, cpu, address);
	lsr_a(bus, cpu, address);

	/*uint8_t operand = cpu_read(bus, address); 

	uint8_t tmp = cpu->a & operand;
	cpu->a >>= 1;

	cpu->p.c = tmp;
	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;*/

	return 0;
}

size_t arr(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);

	uint8_t tmp = cpu->a & operand;
	cpu->a >>= 1;
	
	uint8_t carry = cpu->p.c;
	cpu->p.c = tmp >> 7;
	cpu->p.z = cpu->a == 0;
	cpu->p.v = (tmp >> 7) & (tmp >> 6);
	cpu->p.n = cpu->a >> 7;

	cpu->a = (carry << 7) | (cpu->a & 0x7f);

	return 0;
}

size_t anc(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	and(bus, cpu, address);

	cpu->p.n = cpu->a >> 7;
	
	/*cpu->a &= cpu_read(bus, address);

	cpu->p.c = cpu->a >> 7;
	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;*/
	
	return 0;
}
size_t axs(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);

	uint8_t tmp = cpu->a & cpu->x;
	cpu->x = tmp - operand;

	cpu->p.c = tmp >= operand;
	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t dcp(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	dec(bus, cpu, address);
	cmp(bus, cpu, address);
	
	return 0;
}

size_t isc(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	inc(bus, cpu, address);
	sbc(bus, cpu, address);
	
	return 0;
}

size_t kil(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	printf("kil opcode executed D:.\n");
	exit(EXIT_FAILURE);

	return 0;
}
size_t las(nes_bus* bus, nes_cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);

	uint8_t tmp = operand & cpu->s;
	cpu->a = tmp;
	cpu->x = tmp;
	cpu->s = tmp;

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;

	return 0;
}
size_t lax(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);

	cpu->a = operand;
	cpu->x = operand;

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t rla(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	rol(bus, cpu, address);
	and(bus, cpu, address);

	return 0;
}

size_t rra(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	ror(bus, cpu, address);
	adc(bus, cpu, address);
	
	return 0;
}

size_t sax(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	cpu_write(bus, cpu->a & cpu->x, address);
	
	return 0;
}

size_t slo(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	asl(bus, cpu, address);
	ora(bus, cpu, address);
	
	return 0;
}

size_t sre(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	lsr(bus, cpu, address);
	eor(bus, cpu, address);
	
	return 0;
}

size_t shy(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t hi_before = (cpu->pc - 1) >> 8;

	cpu_write(bus, cpu->y & hi_before, address);
	
	return 0;
}

size_t shx(nes_bus* bus, nes_cpu* cpu, uint16_t address) { 
	
	uint8_t hi_before = (cpu->pc - 1) >> 8;

	cpu_write(bus, cpu->x & hi_before, address);
	
	return 0;
}

size_t tas(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t hi_before = (cpu->pc - 1) >> 8;
	uint8_t tmp = cpu->a & cpu->x;

	cpu->s = tmp;
	cpu_write(bus, tmp & hi_before, address);
	
	return 0;
}

size_t xaa(nes_bus* bus, nes_cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;

	cpu->a = (cpu->a | XAA_CONST) & cpu->x & operand;
	
	return 0;
}

void push(nes_bus* bus, nes_cpu* cpu, uint8_t element) {

	cpu_write(bus, element, STACK_PAGE + cpu->s);
	--cpu->s;
}

uint8_t pop(nes_bus* bus, nes_cpu* cpu) {

	++cpu->s;
	uint8_t element = cpu_read(bus, STACK_PAGE + cpu->s);

	return element;
}

uint8_t colapse_status(nes_cpu* cpu) {
	
	uint8_t status = cpu->p.c | (cpu->p.z << 1) | (cpu->p.i << 2) | (cpu->p.d << 3) | 
					(cpu->p.b << 4) | (cpu->p.v << 6) | (cpu->p.n << 7);

	return status;
}