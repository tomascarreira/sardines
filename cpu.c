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

const size_t (*opcode_table[256])(bus*, cpu*, uint16_t) = {
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

cpu* init_cpu(bus* bus) {
	
	cpu* cpu = calloc(1, sizeof cpu);
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

void clock_cpu(cpu* cpu, bus* bus) {
	
	static size_t instr_clocks;

	if (instr_clocks) {
		--instr_clocks;
		return;
	}
	
	uint8_t opcode = cpu_read(bus, cpu->pc);
	++cpu->pc;

	uint16_t address;

	instr_clocks = opcode_cycles_table[opcode];
	instr_clocks += addressing_mode_table[opcode](bus, cpu, &address);
	instr_clocks += opcode_table[opcode](bus, cpu, address);
	
	--instr_clocks;
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

size_t adc(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint16_t result = operand + cpu->a + cpu->p.c;

	cpu->p.c = result > 0xff;
	cpu->p.z = result == 0x00;
	cpu->p.v = ((uint8_t) result ^ cpu->a) & ((uint8_t) result ^ operand) >> 7;
	cpu->p.n = (int8_t) result < 0; 
	
	return 0;
}

size_t and(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->a &= cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t asl(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint8_t result = operand << 1;

	cpu_write(bus, result, address);

	cpu->p.c = operand >> 7;
	cpu->p.z = result == 0;

	return 0;
}

size_t asl_a(bus* bus, cpu* cpu, uint16_t address) {

	uint8_t operand = cpu->a;

	cpu->a <<= 1;

	cpu->p.c = operand >> 7;
	cpu->p.z = cpu->a == 0;

	return 0;
}

size_t bcc(bus* bus, cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (!cpu->p.c) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0; 
}

size_t bcs(bus* bus, cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (cpu->p.c) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t beq(bus* bus, cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (cpu->p.z) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t bit(bus* bus, cpu* cpu, uint16_t address) {

	uint8_t temp = cpu_read(bus, address) & cpu->a;

	cpu->p.z = temp == 0;
	cpu->p.v = (temp >> 6) & 0x01;
	cpu->p.n = temp >> 7;

	return 0;
}
size_t bmi(bus* bus, cpu* cpu, uint16_t address) {
	
	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (cpu->p.n) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t bne(bus* bus, cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (!cpu->p.z) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t bpl(bus* bus, cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (!cpu->p.n) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t brk(bus* bus, cpu* cpu, uint16_t address) {

	++cpu->pc;
	
	push(bus, cpu, cpu->pc >> 8);
	push(bus, cpu, cpu->pc);

	cpu->p.b = 3;
	push(bus, cpu, colapse_status(cpu));

	cpu->pc = get_irq_vector(bus);
	
	return 0;
}
size_t bvc(bus* bus, cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (!cpu->p.v) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t bvs(bus* bus, cpu* cpu, uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu->pc;

	if (cpu->p.v) {

		cpu->pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t clc(bus* bus, cpu* cpu, uint16_t address) { 

	cpu->p.c = 0;

	return 0;
}
size_t cld(bus* bus, cpu* cpu, uint16_t address) {

	cpu->p.d = 0;

	return 0;
}
size_t cli(bus* bus, cpu* cpu, uint16_t address) {

	cpu->p.i = 0;

	return 0;
}
size_t clv(bus* bus, cpu* cpu, uint16_t address) {

	cpu->p.v = 0;

	return 0;
}
size_t cmp(bus* bus, cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);

	cpu->p.c = cpu->a >= operand;
	cpu->p.z = cpu->a == operand;
	cpu->p.n = (cpu->a - operand) >> 7; 

	return 0;
}
size_t cpx(bus* bus, cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);

	cpu->p.c = cpu->x >= operand;
	cpu->p.z = cpu->x == operand;
	cpu->p.n = (cpu->x - operand) >> 7;
	
	return 0;
}

size_t cpy(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);

	cpu->p.c = cpu->y >= operand;
	cpu->p.z = cpu->y == operand;
	cpu->p.n = (cpu->y - operand) >> 7; 
	
	return 0;
}
size_t dec(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	--operand;

	cpu_write(bus, operand, address);

	cpu->p.z = operand == 0;
	cpu->p.n = operand >> 7;
	
	return 0;
}
size_t dex(bus* bus, cpu* cpu, uint16_t address) {
	
	--cpu->x;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}
size_t dey(bus* bus, cpu* cpu, uint16_t address) {
	
	--cpu->y;

	cpu->p.z = cpu->y == 0;
	cpu->p.n = cpu->y >> 7;
	
	return 0;
}
size_t eor(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->a ^= cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t inc(bus* bus, cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);
	++operand;

	cpu_write(bus, operand, address);

	cpu->p.z = operand == 0;
	cpu->p.n = operand >> 7;
	
	return 0;

	return 0;
}
size_t inx(bus* bus, cpu* cpu, uint16_t address) {
	
	++cpu->x;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t iny(bus* bus, cpu* cpu, uint16_t address) {
	
	++cpu->y;

	cpu->p.z = cpu->y == 0;
	cpu->p.n = cpu->y >> 7;
	
	return 0;
}

size_t jmp(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->pc = address;
	
	return 0;
}

size_t jsr(bus* bus, cpu* cpu, uint16_t address) {
	
	push(bus, cpu, (cpu->pc >> 8) - 1);
	push(bus, cpu, cpu->pc - 1);

	cpu->pc = address;
	
	return 0;
}

size_t lda(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->a = cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t ldx(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->x = cpu_read(bus, address);

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t ldy(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->y = cpu_read(bus, address);

	cpu->p.z = cpu->y == 0;
	cpu->p.n = cpu->y >> 7;
	
	return 0;
}

size_t lsr(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint8_t result = operand >> 1;

	cpu_write(bus, result, address);

	cpu->p.c = operand;
	cpu->p.z = result == 0;
	cpu->p.n = 	result >> 7;
	
	return 0;
}

size_t lsr_a(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);

	cpu->a = operand >> 1;

	cpu->p.c = operand;
	cpu->p.z = cpu->a == 0;
	cpu->p.n = 	cpu->a >> 7;
	
	return 0;
}

size_t nop(bus* bus, cpu* cpu, uint16_t address) {
	
	return 0;
}

size_t ora(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->a |= cpu_read(bus, address);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t pha(bus* bus, cpu* cpu, uint16_t address) {
	
	push(bus, cpu, cpu->a);
	
	return 0;
}

size_t php(bus* bus, cpu* cpu, uint16_t address) {

	cpu->p.b = 3;	
	push(bus, cpu, colapse_status(cpu));
	
	return 0;
}

size_t pla(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->a = pop(bus, cpu);

	cpu->p.z = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;
	
	return 0;
}

size_t plp(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t status = pop(bus, cpu);
	cpu->p.c = status;
	cpu->p.z = status >> 1;
	cpu->p.i = status >> 2;
	cpu->p.d = status >> 3;
	cpu->p.v = status >> 6;
	cpu->p.n = status >> 7;
	
	return 0;
}

size_t rol(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint8_t result = (operand << 1) | cpu->p.c;

	cpu_write(bus, result, address);

	cpu->p.c = operand >> 7;
	cpu->p.c = result == 0;
	cpu->p.n = result >> 7;
	
	return 0;
}

size_t rol_a(bus* bus, cpu* cpu, uint16_t address) {

	uint8_t operand = cpu_read(bus, address);

	cpu->a = (cpu->a << 1) | cpu->p.c;

	cpu->p.c = operand >> 7;
	cpu->p.c = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;	 

	return 0;
}

size_t ror(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);
	uint8_t result = (operand >> 1) | (cpu->p.c << 7);

	cpu_write(bus, result, address);

	cpu->p.c = operand;
	cpu->p.c = result == 0;
	cpu->p.n = result >> 7;
	
	return 0;
}

size_t ror_a(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = cpu_read(bus, address);

	cpu->a = (cpu->a >> 1) | (cpu->p.c << 7);

	cpu->p.c = operand;
	cpu->p.c = cpu->a == 0;
	cpu->p.n = cpu->a >> 7;	
	
	return 0;
}

size_t rti(bus* bus, cpu* cpu, uint16_t address) {
	
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

size_t rts(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t lo = pop(bus, cpu);
	uint8_t hi = pop(bus, cpu);

	cpu->pc = (hi << 8) | lo;
	++cpu->pc;
	
	return 0;
}

size_t sbc(bus* bus, cpu* cpu, uint16_t address) {
	
	uint8_t operand = ~cpu_read(bus, address);
	uint16_t result = operand + cpu->a + cpu->p.c;

	cpu->p.c = result > 0xff;
	cpu->p.z = result == 0x00;
	cpu->p.v = ((uint8_t) result ^ cpu->a) & ((uint8_t) result ^ operand) >> 7;
	cpu->p.n = (int8_t) result < 0; 
	
	return 0;
	
	return 0; 
}

size_t sec(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->p.c = 1;
	
	return 0;
}

size_t sed(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->p.d = 1;
	
	return 0;
}

size_t sei(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->p.i = 1;
	
	return 0;
}

size_t sta(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu_write(bus, cpu->a, address);
	
	return 0;
}

size_t stx(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu_write(bus, cpu->x, address);
	
	return 0;
}

size_t sty(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu_write(bus, cpu->y, address);

	return 0;
}

size_t tax(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->x = cpu->a;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t tay(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->y = cpu->a;

	cpu->p.z = cpu->y == 0;
	cpu->p.n = cpu->y >> 7;
	
	return 0;
}

size_t tsx(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->x = cpu->s;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t txa(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->a = cpu->x;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t txs(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->s = cpu->x;

	return 0;
}

size_t tya(bus* bus, cpu* cpu, uint16_t address) {
	
	cpu->a = cpu->y;

	cpu->p.z = cpu->x == 0;
	cpu->p.n = cpu->x >> 7;
	
	return 0;
}

size_t ahx(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t alr(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t arr(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t anc(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t axs(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t dcp(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t isc(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t kil(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t las(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t lax(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t rla(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t rra(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t sax(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t slo(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t sre(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t shy(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t shx(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t tas(bus* bus, cpu* cpu, uint16_t address) { return 0; }
size_t xaa(bus* bus, cpu* cpu, uint16_t address) { return 0; }

void push(bus* bus, cpu* cpu, uint8_t element) {

	cpu_write(bus, element, STACK_PAGE + cpu->s);
	--cpu->s;
}

uint8_t pop(bus* bus, cpu* cpu) {

	++cpu->s;
	uint8_t element = cpu_read(bus, STACK_PAGE + cpu->s);

	return element;
}

uint8_t colapse_status(cpu* cpu) {
	
	uint8_t status = cpu->p.c | (cpu->p.z << 1) | (cpu->p.i << 2) | (cpu->p.d << 3) | 
					(cpu->p.b << 4) | (cpu->p.v << 6) | (cpu->p.n << 7);

	return status;
}