#ifndef CPU_H
#define CPU_H

#define STACK_POINTER 0xfd
#define STACK_PAGE 0x100

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
		uint8_t v:1;
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

size_t adc(bus* bus, cpu* cpu, uint16_t address);
size_t and(bus* bus, cpu* cpu, uint16_t address);
size_t asl(bus* bus, cpu* cpu, uint16_t address);
size_t asl_a(bus* bus, cpu* cpu, uint16_t address);
size_t bcc(bus* bus, cpu* cpu, uint16_t address);
size_t bcs(bus* bus, cpu* cpu, uint16_t address);
size_t beq(bus* bus, cpu* cpu, uint16_t address);
size_t bit(bus* bus, cpu* cpu, uint16_t address);
size_t bmi(bus* bus, cpu* cpu, uint16_t address);
size_t bne(bus* bus, cpu* cpu, uint16_t address);
size_t bpl(bus* bus, cpu* cpu, uint16_t address);
size_t brk(bus* bus, cpu* cpu, uint16_t address);
size_t bvc(bus* bus, cpu* cpu, uint16_t address);
size_t bvs(bus* bus, cpu* cpu, uint16_t address);
size_t clc(bus* bus, cpu* cpu, uint16_t address);
size_t cld(bus* bus, cpu* cpu, uint16_t address);
size_t cli(bus* bus, cpu* cpu, uint16_t address);
size_t clv(bus* bus, cpu* cpu, uint16_t address);
size_t cmp(bus* bus, cpu* cpu, uint16_t address);
size_t cpx(bus* bus, cpu* cpu, uint16_t address);
size_t cpy(bus* bus, cpu* cpu, uint16_t address);
size_t dec(bus* bus, cpu* cpu, uint16_t address);
size_t dex(bus* bus, cpu* cpu, uint16_t address);
size_t dey(bus* bus, cpu* cpu, uint16_t address);
size_t eor(bus* bus, cpu* cpu, uint16_t address);
size_t inc(bus* bus, cpu* cpu, uint16_t address);
size_t inx(bus* bus, cpu* cpu, uint16_t address);
size_t iny(bus* bus, cpu* cpu, uint16_t address);
size_t jmp(bus* bus, cpu* cpu, uint16_t address);
size_t jsr(bus* bus, cpu* cpu, uint16_t address);
size_t lda(bus* bus, cpu* cpu, uint16_t address);
size_t ldx(bus* bus, cpu* cpu, uint16_t address);
size_t ldy(bus* bus, cpu* cpu, uint16_t address);
size_t lsr(bus* bus, cpu* cpu, uint16_t address);
size_t lsr_a(bus* bus, cpu* cpu, uint16_t address);
size_t nop(bus* bus, cpu* cpu, uint16_t address);
size_t ora(bus* bus, cpu* cpu, uint16_t address);
size_t pha(bus* bus, cpu* cpu, uint16_t address);
size_t php(bus* bus, cpu* cpu, uint16_t address);
size_t pla(bus* bus, cpu* cpu, uint16_t address);
size_t plp(bus* bus, cpu* cpu, uint16_t address);
size_t rol(bus* bus, cpu* cpu, uint16_t address);
size_t rol_a(bus* bus, cpu* cpu, uint16_t address);
size_t ror(bus* bus, cpu* cpu, uint16_t address);
size_t ror_a(bus* bus, cpu* cpu, uint16_t address);
size_t rti(bus* bus, cpu* cpu, uint16_t address);
size_t rts(bus* bus, cpu* cpu, uint16_t address);
size_t sbc(bus* bus, cpu* cpu, uint16_t address);
size_t sec(bus* bus, cpu* cpu, uint16_t address);
size_t sed(bus* bus, cpu* cpu, uint16_t address);
size_t sei(bus* bus, cpu* cpu, uint16_t address);
size_t sta(bus* bus, cpu* cpu, uint16_t address);
size_t stx(bus* bus, cpu* cpu, uint16_t address);
size_t sty(bus* bus, cpu* cpu, uint16_t address);
size_t tax(bus* bus, cpu* cpu, uint16_t address);
size_t tay(bus* bus, cpu* cpu, uint16_t address);
size_t tsx(bus* bus, cpu* cpu, uint16_t address);
size_t txa(bus* bus, cpu* cpu, uint16_t address);
size_t txs(bus* bus, cpu* cpu, uint16_t address);
size_t tya(bus* bus, cpu* cpu, uint16_t address);

size_t ahx(bus* bus, cpu* cpu, uint16_t address);
size_t alr(bus* bus, cpu* cpu, uint16_t address);
size_t arr(bus* bus, cpu* cpu, uint16_t address);
size_t anc(bus* bus, cpu* cpu, uint16_t address);
size_t axs(bus* bus, cpu* cpu, uint16_t address);
size_t dcp(bus* bus, cpu* cpu, uint16_t address);
size_t isc(bus* bus, cpu* cpu, uint16_t address);
size_t kil(bus* bus, cpu* cpu, uint16_t address);
size_t las(bus* bus, cpu* cpu, uint16_t address);
size_t lax(bus* bus, cpu* cpu, uint16_t address);
size_t rla(bus* bus, cpu* cpu, uint16_t address);
size_t rra(bus* bus, cpu* cpu, uint16_t address);
size_t sax(bus* bus, cpu* cpu, uint16_t address);
size_t slo(bus* bus, cpu* cpu, uint16_t address);
size_t sre(bus* bus, cpu* cpu, uint16_t address);
size_t shy(bus* bus, cpu* cpu, uint16_t address);
size_t shx(bus* bus, cpu* cpu, uint16_t address);
size_t tas(bus* bus, cpu* cpu, uint16_t address);
size_t xaa(bus* bus, cpu* cpu, uint16_t address);

void push(bus* bus, cpu* cpu, uint8_t element);
uint8_t pop(bus* bus, cpu* cpu);
uint8_t colapse_status(cpu* cpu);

#endif