#include "common.h"
#include "cpu.h"
#include "ppu.h"
#include "cartridge.h"
#include "log.h"

static nes_cpu cpu = { 0 };
static uint8_t* ram;
static uint8_t oamdma = 0;

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
		2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
};

size_t (*addressing_mode_table[256])(uint16_t*, uint8_t) = {
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

size_t (*opcode_table[256])(uint16_t) = {
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

void init_cpu() {
	
	cpu.s = STACK_POINTER;
	cpu.p.i = 1;
	cpu.p.b = 3;
	cpu.pc = get_reset_vector();
}

void init_ram(void) {
	
	ram = calloc(0x800, 1);
	if (!ram) {
		perror("ram calloc failed\n");
		exit(EXIT_FAILURE);
	}
}

void clock_cpu() {
	
	static size_t instr_clocks;

	if (instr_clocks) {
		--instr_clocks;
		return;
	}
	
	//log_instr(cpu);

	uint8_t opcode = cpu_read(cpu.pc);
	++cpu.pc;

	uint16_t address;

	instr_clocks = opcode_cycles_table[opcode];
	instr_clocks += addressing_mode_table[opcode](&address, opcode);
	instr_clocks += opcode_table[opcode](address);
	
	--instr_clocks;
}

uint8_t cpu_read(uint16_t address) {
	
	uint8_t value;
	if (address <= 0x1fff) {
		value = ram[address & 0x7ff];

	} else if (address >= 0x2000 && address <= 0x3fff) {
		value = ppu_registers_read(address);

	} else if (address >= 0x4000 && address <= 0x401f){


	} else if (address >= 0x4020 && address <= 0xffff) {
		value = mapper_read(address);
	
	} else {
		printf("Read bus address %02x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}

	return value;
}

void cpu_write(uint8_t value, uint16_t address) {
	
	if (address <= 0x7ff) {
		ram[address & 0x7ff] = value;

	} else if (address >= 0x2000 && address <= 0x3fff) {
		ppu_registers_write(value, address);

	} else if (address >= 0x4000 && address <= 0x401f){
		if (address == 0x4014) {
			oamdma = value;
			uint8_t* page_ptr = NULL;  // TODO
			ppu_oamdma(page_ptr);
			cycles += cycles % 2 ? 513 : 514; // when odd cpu cycle add 1 cycle
		} else {

		}

	} else if (address >= 0x4020 && address <= 0xffff) {
		mapper_write(value, address);

	} else {
		printf("Write bus address %02x not implemented.\n", address);
		exit(EXIT_FAILURE);
	}
}

uint8_t log_read_ram(uint16_t address) {

	return ram[address & 0x7ff];
}

void push(uint8_t element) {

	cpu_write(element, STACK_PAGE + cpu.s);
	--cpu.s;
}

uint8_t pop() {

	++cpu.s;
	uint8_t element = cpu_read(STACK_PAGE + cpu.s);

	return element;
}

uint8_t colapse_status() {
	
	uint8_t status = cpu.p.c | (cpu.p.z << 1) | (cpu.p.i << 2) | (cpu.p.d << 3) | 
					(cpu.p.b << 4) | (cpu.p.v << 6) | (cpu.p.n << 7);

	return status;
}

void nmi() {

	push(cpu.pc >> 8);
	push(cpu.pc >> 8);
	push(colapse_status(cpu));

	cpu.pc = get_nmi_vector();
}

void irq() {

	push(cpu.pc >> 8);
	push(cpu.pc >> 8);
	push(colapse_status(cpu));

	cpu.pc = get_irq_vector();
}

void reset() {

	cpu.s -= 3;
	cpu.p.i = 1;
	cpu.pc = get_reset_vector();
}

void free_ram(void) {

	free(ram);
}

size_t imp(uint16_t* address, uint8_t opcode) {
	
	return 0;
}

size_t imm(uint16_t* address, uint8_t opcode) {
	
	*address = cpu.pc;
	++cpu.pc;

	return 0;
}

size_t zp(uint16_t* address, uint8_t opcode) {
	
	*address = cpu_read(cpu.pc);
	++cpu.pc;

	return 0;
}

size_t zpx(uint16_t* address, uint8_t opcode) {

	uint8_t operand =	cpu_read(cpu.pc);
	++cpu.pc;

	*address = (operand + cpu.x) & 0xff;

	return 0;
}

size_t zpy(uint16_t* address, uint8_t opcode) {

	uint8_t operand =	cpu_read(cpu.pc);
	++cpu.pc;

	*address = (operand + cpu.y) & 0xff;

	return 0;
}

size_t abl(uint16_t* address, uint8_t opcode) {

	uint8_t lo = cpu_read(cpu.pc);
	++cpu.pc;

	uint8_t hi = cpu_read(cpu.pc);
	++cpu.pc;

	*address = (hi << 8) | lo;

	return 0;
}

size_t abx(uint16_t* address, uint8_t opcode) {

	uint8_t lo = cpu_read(cpu.pc);
	++cpu.pc;

	uint8_t hi = cpu_read(cpu.pc);
	++cpu.pc;

	*address = ((hi << 8) | lo) + cpu.x;

	if ((lo + cpu.x > 0xff) & ((opcode == 0x1c) | (opcode == 0x1d) | (opcode == 0x3c) |
		(opcode == 0x3d) | (opcode == 0x5c) | (opcode == 0x5d) | (opcode == 0x7c) |
		(opcode == 0x7d) | (opcode == 0xbc) | (opcode == 0xbd) | (opcode == 0xdc) |
		(opcode == 0xdd) | (opcode == 0xfc) | (opcode == 0xfd))) {
		return 1;
	}

	return 0;
}

size_t aby(uint16_t* address, uint8_t opcode) {

	uint8_t lo = cpu_read(cpu.pc);
	++cpu.pc;

	uint8_t hi = cpu_read(cpu.pc);
	++cpu.pc;

	*address = ((hi << 8) | lo) + cpu.y;

	if ((lo + cpu.y > 0xff) & ((opcode == 0x1a) | (opcode == 0x39) | (opcode == 0x59) |
		(opcode == 0x79) | (opcode == 0xb9) | (opcode == 0xbb) | (opcode == 0xbe) |
		(opcode == 0xbf) | (opcode == 0xd9) | (opcode == 0xf9))) {
		return 1;
	}

	return 0;
}

size_t rel(uint16_t* address, uint8_t opcode) {
	
	*address = cpu_read(cpu.pc);
	++cpu.pc;

	return 0;
}

size_t ind(uint16_t* address, uint8_t opcode) {
	
	uint8_t lo = cpu_read(cpu.pc);
	++cpu.pc;

	uint8_t hi = cpu_read(cpu.pc);
	++cpu.pc;

	uint16_t ptr = (hi << 8) | lo;

	if (lo == 0xff) {
	   *address = (cpu_read(ptr & 0xff00) << 8) | cpu_read(ptr);

	} else {
		*address = (cpu_read(ptr + 1) << 8) | cpu_read(ptr);
	}

	return 0;
}

size_t izx(uint16_t* address, uint8_t opcode) {
	
	uint8_t zp_ptr = cpu_read(cpu.pc);
	++cpu.pc;

	uint8_t lo = cpu_read((zp_ptr + cpu.x) & 0xff);
	uint8_t hi = cpu_read((zp_ptr + cpu.x + 1) & 0xff);

	*address = (hi << 8) | lo;

	return 0;
}

size_t izy(uint16_t* address, uint8_t opcode) {
	
	uint8_t zp_ptr = cpu_read(cpu.pc);
	++cpu.pc;

	uint8_t lo = cpu_read(zp_ptr);
	uint8_t hi = cpu_read((zp_ptr + 1) & 0xff);

	*address = ((hi << 8) | lo) + cpu.y;

	if ((lo + cpu.y > 0xff) & ((opcode == 0x11) | (opcode == 0x31) | (opcode == 0x51) |
		(opcode == 0x71) | (opcode == 0xb1) | (opcode == 0xb3) | (opcode == 0xd1) |
		(opcode == 0xf1))) {
		return 1;
	}

	return 0;
}

size_t adc(uint16_t address) {
	
	uint8_t operand = cpu_read(address);
	uint16_t result = operand + cpu.a + cpu.p.c;

	cpu.p.c = result > 0xff;
	cpu.p.z = (uint8_t) result == 0x00;
	cpu.p.v = (((uint8_t) result ^ cpu.a) & ((uint8_t) result ^ operand)) >> 7;
	cpu.p.n = (int8_t) result < 0;

	cpu.a = result;
	
	return 0;
}

size_t and(uint16_t address) {
	
	cpu.a &= cpu_read(address);

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;
	
	return 0;
}

size_t asl(uint16_t address) {
	
	uint8_t operand = cpu_read(address);
	uint8_t result = operand << 1;

	cpu_write(result, address);

	cpu.p.c = operand >> 7;
	cpu.p.z = result == 0;
	cpu.p.n = (int8_t) result < 0;

	return 0;
}

size_t asl_a(uint16_t address) {

	uint8_t operand = cpu.a;

	cpu.a <<= 1;

	cpu.p.c = operand >> 7;
	cpu.p.z = cpu.a == 0;
	cpu.p.n = (int8_t) cpu.a < 0;

	return 0;
}

size_t bcc(uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu.pc;

	if (!cpu.p.c) {

		cpu.pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0; 
}

size_t bcs(uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu.pc;

	if (cpu.p.c) {

		cpu.pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t beq(uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu.pc;

	if (cpu.p.z) {

		cpu.pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t bit(uint16_t address) {

	uint8_t operand = cpu_read(address);

	uint8_t tmp = operand & cpu.a;

	cpu.p.z = tmp == 0;
	cpu.p.v = (operand >> 6) & 0x01;
	cpu.p.n = operand >> 7;

	return 0;
}
size_t bmi(uint16_t address) {
	
	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu.pc;

	if (cpu.p.n) {

		cpu.pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t bne(uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu.pc;

	if (!cpu.p.z) {

		cpu.pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t bpl(uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu.pc;

	if (!cpu.p.n) {

		cpu.pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t brk(uint16_t address) {

	++cpu.pc;
	
	push(cpu.pc >> 8);
	push(cpu.pc);

	cpu.p.b = 3;
	push(colapse_status());

	cpu.pc = get_irq_vector();

	cpu.p.i = 1;
	
	return 0;
}
size_t bvc(uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu.pc;

	if (!cpu.p.v) {

		cpu.pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}

size_t bvs(uint16_t address) {

	int8_t offset = (int8_t) address;
	uint16_t old_pc = cpu.pc;

	if (cpu.p.v) {

		cpu.pc += offset;

		return ((uint8_t) old_pc + offset > 0xff) ? 2 : 1;
	}

	return 0;
}
size_t clc(uint16_t address) { 

	cpu.p.c = 0;

	return 0;
}
size_t cld(uint16_t address) {

	cpu.p.d = 0;

	return 0;
}
size_t cli(uint16_t address) {

	cpu.p.i = 0;

	return 0;
}
size_t clv(uint16_t address) {

	cpu.p.v = 0;

	return 0;
}
size_t cmp(uint16_t address) {

	uint8_t operand = cpu_read(address);

	cpu.p.c = cpu.a >= operand;
	cpu.p.z = cpu.a == operand;
	cpu.p.n = (cpu.a - operand) >> 7; 

	return 0;
}
size_t cpx(uint16_t address) {

	uint8_t operand = cpu_read(address);

	cpu.p.c = cpu.x >= operand;
	cpu.p.z = cpu.x == operand;
	cpu.p.n = (cpu.x - operand) >> 7;
	
	return 0;
}

size_t cpy(uint16_t address) {
	
	uint8_t operand = cpu_read(address);

	cpu.p.c = cpu.y >= operand;
	cpu.p.z = cpu.y == operand;
	cpu.p.n = (cpu.y - operand) >> 7; 
	
	return 0;
}
size_t dec(uint16_t address) {
	
	uint8_t operand = cpu_read(address);
	--operand;

	cpu_write(operand, address);

	cpu.p.z = operand == 0;
	cpu.p.n = operand >> 7;
	
	return 0;
}
size_t dex(uint16_t address) {
	
	--cpu.x;

	cpu.p.z = cpu.x == 0;
	cpu.p.n = cpu.x >> 7;
	
	return 0;
}
size_t dey(uint16_t address) {
	
	--cpu.y;

	cpu.p.z = cpu.y == 0;
	cpu.p.n = cpu.y >> 7;
	
	return 0;
}
size_t eor(uint16_t address) {
	
	cpu.a ^= cpu_read(address);

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;
	
	return 0;
}

size_t inc(uint16_t address) {

	uint8_t operand = cpu_read(address);
	++operand;

	cpu_write(operand, address);

	cpu.p.z = operand == 0;
	cpu.p.n = operand >> 7;
	
	return 0;

	return 0;
}
size_t inx(uint16_t address) {
	
	++cpu.x;

	cpu.p.z = cpu.x == 0;
	cpu.p.n = cpu.x >> 7;
	
	return 0;
}

size_t iny(uint16_t address) {
	
	++cpu.y;

	cpu.p.z = cpu.y == 0;
	cpu.p.n = cpu.y >> 7;
	
	return 0;
}

size_t jmp(uint16_t address) {
	
	cpu.pc = address;
	
	return 0;
}

size_t jsr(uint16_t address) {
	
	push((cpu.pc - 1) >> 8);
	push(cpu.pc - 1);

	cpu.pc = address;
	
	return 0;
}

size_t lda(uint16_t address) {
	
	cpu.a = cpu_read(address);

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;
	
	return 0;
}

size_t ldx(uint16_t address) {
	
	cpu.x = cpu_read(address);

	cpu.p.z = cpu.x == 0;
	cpu.p.n = cpu.x >> 7;
	
	return 0;
}

size_t ldy(uint16_t address) {
	
	cpu.y = cpu_read(address);

	cpu.p.z = cpu.y == 0;
	cpu.p.n = cpu.y >> 7;
	
	return 0;
}

size_t lsr(uint16_t address) {
	
	uint8_t operand = cpu_read(address);
	uint8_t result = operand >> 1;

	cpu_write(result, address);

	cpu.p.c = operand;
	cpu.p.z = result == 0;
	cpu.p.n = 	result >> 7;
	
	return 0;
}

size_t lsr_a(uint16_t address) {
	
	uint8_t operand = cpu.a;

	cpu.a = operand >> 1;

	cpu.p.c = operand;
	cpu.p.z = cpu.a == 0;
	cpu.p.n = 	cpu.a >> 7;
	
	return 0;
}

size_t nop(uint16_t address) {
	
	return 0;
}

size_t ora(uint16_t address) {
	
	cpu.a |= cpu_read(address);

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;
	
	return 0;
}

size_t pha(uint16_t address) {
	
	push(cpu.a);
	
	return 0;
}

size_t php(uint16_t address) {
	
	uint8_t b_flag = cpu.p.b;

	cpu.p.b = 3;	
	push(colapse_status(cpu));
	cpu.p.b = b_flag;
	
	return 0;
}

size_t pla(uint16_t address) {
	
	cpu.a = pop(cpu);

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;
	
	return 0;
}

size_t plp(uint16_t address) {
	
	uint8_t status = pop(cpu);
	cpu.p.c = status;
	cpu.p.z = status >> 1;
	cpu.p.i = status >> 2;
	cpu.p.d = status >> 3;
	cpu.p.v = status >> 6;
	cpu.p.n = status >> 7;
	
	return 0;
}

size_t rol(uint16_t address) {
	
	uint8_t operand = cpu_read(address);
	uint8_t result = (operand << 1) | cpu.p.c;

	cpu_write(result, address);

	cpu.p.c = operand >> 7;
	cpu.p.z = result == 0;
	cpu.p.n = result >> 7;
	
	return 0;
}

size_t rol_a(uint16_t address) {

	uint8_t operand = cpu.a;

	cpu.a = (cpu.a << 1) | cpu.p.c;

	cpu.p.c = operand >> 7;
	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;	 

	return 0;
}

size_t ror(uint16_t address) {
	
	uint8_t operand = cpu_read(address);
	uint8_t result = (operand >> 1) | (cpu.p.c << 7);

	cpu_write(result, address);

	cpu.p.c = operand;
	cpu.p.z = result == 0;
	cpu.p.n = result >> 7;
	
	return 0;
}

size_t ror_a(uint16_t address) {
	
	uint8_t operand = cpu.a;

	cpu.a = (cpu.a >> 1) | (cpu.p.c << 7);

	cpu.p.c = operand;
	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;	
	
	return 0;
}

size_t rti(uint16_t address) {
	
	uint8_t status = pop(cpu);
	cpu.p.c = status;
	cpu.p.z = status >> 1;
	cpu.p.i = status >> 2;
	cpu.p.d = status >> 3;
	cpu.p.v = status >> 6;
	cpu.p.n = status >> 7;

	uint8_t lo = pop(cpu);
	uint8_t hi = pop(cpu);

	cpu.pc = (hi << 8) | lo;
	
	return 0;
}

size_t rts(uint16_t address) {
	
	uint8_t lo = pop(cpu);
	uint8_t hi = pop(cpu);

	cpu.pc = (hi << 8) | lo;
	++cpu.pc;
	
	return 0;
}

size_t sbc(uint16_t address) {
	
	uint8_t operand = ~cpu_read(address);
	uint16_t result = operand + cpu.a + cpu.p.c;

	cpu.p.c = result > 0xff;
	cpu.p.z = (uint8_t) result == 0x00;
	cpu.p.v = (((uint8_t) result ^ cpu.a) & ((uint8_t) result ^ operand)) >> 7;
	cpu.p.n = (int8_t) result < 0; 

	cpu.a = result;
	
	return 0; 
}

size_t sec(uint16_t address) {
	
	cpu.p.c = 1;
	
	return 0;
}

size_t sed(uint16_t address) {
	
	cpu.p.d = 1;
	
	return 0;
}

size_t sei(uint16_t address) {
	
	cpu.p.i = 1;
	
	return 0;
}

size_t sta(uint16_t address) {
	
	cpu_write(cpu.a, address);
	
	return 0;
}

size_t stx(uint16_t address) {
	
	cpu_write(cpu.x, address);
	
	return 0;
}

size_t sty(uint16_t address) {
	
	cpu_write(cpu.y, address);

	return 0;
}

size_t tax(uint16_t address) {
	
	cpu.x = cpu.a;

	cpu.p.z = cpu.x == 0;
	cpu.p.n = cpu.x >> 7;
	
	return 0;
}

size_t tay(uint16_t address) {
	
	cpu.y = cpu.a;

	cpu.p.z = cpu.y == 0;
	cpu.p.n = cpu.y >> 7;
	
	return 0;
}

size_t tsx(uint16_t address) {
	
	cpu.x = cpu.s;

	cpu.p.z = cpu.x == 0;
	cpu.p.n = cpu.x >> 7;
	
	return 0;
}

size_t txa(uint16_t address) {
	
	cpu.a = cpu.x;

	cpu.p.z = cpu.x == 0;
	cpu.p.n = cpu.x >> 7;
	
	return 0;
}

size_t txs(uint16_t address) {
	
	cpu.s = cpu.x;

	return 0;
}

size_t tya(uint16_t address) {
	
	cpu.a = cpu.y;

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;
	
	return 0;
}

size_t ahx(uint16_t address) {

	uint8_t hi_before = (cpu.pc - 1) >> 8;

	cpu_write(cpu.a & cpu.x & (hi_before + 1), address);

	return 0;
}

size_t alr(uint16_t address) {

	and(address);
	lsr_a(address);

	return 0;
}

size_t arr(uint16_t address) {
	
	uint8_t operand = cpu_read(address);
	
	uint8_t tmp = (cpu.a & operand) >> 1;
	cpu.a = (tmp & 0x7f) | (cpu.p.c << 7);  

	cpu.p.c = cpu.a >> 6;	
	cpu.p.z = cpu.a == 0;
	cpu.p.v = (cpu.a >> 6) ^ (cpu.a >> 5);
	cpu.p.n = cpu.a >> 7;

	return 0;
}

size_t anc(uint16_t address) {
	
	and(address);

	cpu.p.c = cpu.a >> 7;

	
	return 0;
}
size_t axs(uint16_t address) {
	
	uint8_t operand = cpu_read(address);

	uint8_t tmp = cpu.a & cpu.x;
	cpu.x = tmp - operand;

	cpu.p.c = tmp >= operand;
	cpu.p.z = cpu.x == 0;
	cpu.p.n = cpu.x >> 7;
	
	return 0;
}

size_t dcp(uint16_t address) {
	
	dec(address);
	cmp(address);
	
	return 0;
}

size_t isc(uint16_t address) {
	
	inc(address);
	sbc(address);
	
	return 0;
}

size_t kil(uint16_t address) {

	printf("kil opcode executed D:.\n");
	exit(EXIT_FAILURE);
}
size_t las(uint16_t address) {

	uint8_t operand = cpu_read(address);

	uint8_t tmp = operand & cpu.s;
	cpu.a = tmp;
	cpu.x = tmp;
	cpu.s = tmp;

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;

	return 0;
}
size_t lax(uint16_t address) {
	
	uint8_t operand = cpu_read(address);

	cpu.a = operand;
	cpu.x = operand;

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;
	
	return 0;
}

size_t rla(uint16_t address) {
	
	rol(address);
	and(address);

	return 0;
}

size_t rra(uint16_t address) {
	
	ror(address);
	adc(address);
	
	return 0;
}

size_t sax(uint16_t address) {
	
	cpu_write(cpu.a & cpu.x, address);
	
	return 0;
}

size_t slo(uint16_t address) {
	
	asl(address);
	ora(address);
	
	return 0;
}

size_t sre(uint16_t address) {
	
	lsr(address);
	eor(address);
	
	return 0;
}

size_t shy(uint16_t address) {
	
	if (((address - cpu.x) ^ address) & 0x100) {
		address = (address & (cpu.y << 8)) | (address & 0x00ff);
	}

	cpu_write(cpu.y & (((address - cpu.x) >> 8) + 1), address);
	
	return 0;
}

size_t shx(uint16_t address) { 
	
	if (((address - cpu.y) ^ address) & 0x100) {
		address = (address & (cpu.x << 8)) | (address & 0x00ff);
	}

	cpu_write(cpu.x & (((address - cpu.y) >> 8) + 1), address);
	
	return 0;
}

size_t tas(uint16_t address) {
	
	uint8_t hi_before = (cpu.pc - 1) >> 8;
	uint8_t tmp = cpu.a & cpu.x;

	cpu.s = tmp;
	cpu_write(tmp & hi_before, address);
	
	return 0;
}

size_t xaa(uint16_t address) {
	
	uint8_t operand = cpu_read(address);

	cpu.p.z = cpu.a == 0;
	cpu.p.n = cpu.a >> 7;

	cpu.a = (cpu.a | XAA_CONST) & cpu.x & operand;
	
	return 0;
}