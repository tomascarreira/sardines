#ifndef CPU_H
#define CPU_H

#define STACK_POINTER 0xfd
#define STACK_PAGE 0x100
#define XAA_CONST 0xff // chip and/or temperature dependent used only for ilegal opcode xaa

typedef struct nes_cpu nes_cpu;
struct nes_cpu {
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
		uint8_t v:1;
		uint8_t n:1;
	} p;
};

nes_cpu* init_cpu(nes_bus* bus);
void clock_cpu(nes_cpu* cpu, nes_bus* bus);
uint8_t cpu_read(nes_bus* bus, uint16_t address);
void cpu_write(nes_bus* bus, uint8_t value, uint16_t address);

size_t imp(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t imm(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t zp(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t zpx(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t zpy(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t abl(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t abx(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t aby(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t rel(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t ind(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t izx(nes_bus* bus, nes_cpu* cpu, uint16_t* address);
size_t izy(nes_bus* bus, nes_cpu* cpu, uint16_t* address);

size_t adc(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t and(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t asl(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t asl_a(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t bcc(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t bcs(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t beq(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t bit(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t bmi(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t bne(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t bpl(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t brk(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t bvc(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t bvs(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t clc(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t cld(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t cli(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t clv(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t cmp(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t cpx(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t cpy(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t dec(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t dex(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t dey(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t eor(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t inc(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t inx(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t iny(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t jmp(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t jsr(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t lda(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t ldx(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t ldy(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t lsr(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t lsr_a(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t nop(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t ora(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t pha(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t php(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t pla(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t plp(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t rol(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t rol_a(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t ror(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t ror_a(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t rti(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t rts(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t sbc(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t sec(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t sed(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t sei(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t sta(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t stx(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t sty(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t tax(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t tay(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t tsx(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t txa(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t txs(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t tya(nes_bus* bus, nes_cpu* cpu, uint16_t address);

size_t ahx(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t alr(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t arr(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t anc(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t axs(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t dcp(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t isc(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t kil(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t las(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t lax(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t rla(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t rra(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t sax(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t slo(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t sre(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t shy(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t shx(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t tas(nes_bus* bus, nes_cpu* cpu, uint16_t address);
size_t xaa(nes_bus* bus, nes_cpu* cpu, uint16_t address);

void push(nes_bus* bus, nes_cpu* cpu, uint8_t element);
uint8_t pop(nes_bus* bus, nes_cpu* cpu);
uint8_t colapse_status(nes_cpu* cpu);

#endif