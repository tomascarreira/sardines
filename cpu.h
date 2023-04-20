#ifndef CPU_H
#define CPU_H

#include <stdlib.h>
#include <stdint.h>

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

void init_cpu(void);
void init_ram(void);
void clock_cpu(void);
uint8_t cpu_read(uint16_t address);
void cpu_write(uint8_t value, uint16_t address);

uint8_t log_read_ram(uint16_t address);

void push(uint8_t element);
uint8_t pop(void);
uint8_t colapse_status(void);

void nmi(void);
void irq(void);
void reset(void);

void free_ram(void);

size_t imp(uint16_t* address, uint8_t opcode);
size_t imm(uint16_t* address, uint8_t opcode);
size_t zp(uint16_t* address, uint8_t opcode);
size_t zpx(uint16_t* address, uint8_t opcode);
size_t zpy(uint16_t* address, uint8_t opcode);
size_t abl(uint16_t* address, uint8_t opcode);
size_t abx(uint16_t* address, uint8_t opcode);
size_t aby(uint16_t* address, uint8_t opcode);
size_t rel(uint16_t* address, uint8_t opcode);
size_t ind(uint16_t* address, uint8_t opcode);
size_t izx(uint16_t* address, uint8_t opcode);
size_t izy(uint16_t* address, uint8_t opcode);

size_t adc(uint16_t address);
size_t and(uint16_t address);
size_t asl(uint16_t address);
size_t asl_a(uint16_t address);
size_t bcc(uint16_t address);
size_t bcs(uint16_t address);
size_t beq(uint16_t address);
size_t bit(uint16_t address);
size_t bmi(uint16_t address);
size_t bne(uint16_t address);
size_t bpl(uint16_t address);
size_t brk(uint16_t address);
size_t bvc(uint16_t address);
size_t bvs(uint16_t address);
size_t clc(uint16_t address);
size_t cld(uint16_t address);
size_t cli(uint16_t address);
size_t clv(uint16_t address);
size_t cmp(uint16_t address);
size_t cpx(uint16_t address);
size_t cpy(uint16_t address);
size_t dec(uint16_t address);
size_t dex(uint16_t address);
size_t dey(uint16_t address);
size_t eor(uint16_t address);
size_t inc(uint16_t address);
size_t inx(uint16_t address);
size_t iny(uint16_t address);
size_t jmp(uint16_t address);
size_t jsr(uint16_t address);
size_t lda(uint16_t address);
size_t ldx(uint16_t address);
size_t ldy(uint16_t address);
size_t lsr(uint16_t address);
size_t lsr_a(uint16_t address);
size_t nop(uint16_t address);
size_t ora(uint16_t address);
size_t pha(uint16_t address);
size_t php(uint16_t address);
size_t pla(uint16_t address);
size_t plp(uint16_t address);
size_t rol(uint16_t address);
size_t rol_a(uint16_t address);
size_t ror(uint16_t address);
size_t ror_a(uint16_t address);
size_t rti(uint16_t address);
size_t rts(uint16_t address);
size_t sbc(uint16_t address);
size_t sec(uint16_t address);
size_t sed(uint16_t address);
size_t sei(uint16_t address);
size_t sta(uint16_t address);
size_t stx(uint16_t address);
size_t sty(uint16_t address);
size_t tax(uint16_t address);
size_t tay(uint16_t address);
size_t tsx(uint16_t address);
size_t txa(uint16_t address);
size_t txs(uint16_t address);
size_t tya(uint16_t address);

size_t ahx(uint16_t address);
size_t alr(uint16_t address);
size_t arr(uint16_t address);
size_t anc(uint16_t address);
size_t axs(uint16_t address);
size_t dcp(uint16_t address);
size_t isc(uint16_t address);
size_t kil(uint16_t address);
size_t las(uint16_t address);
size_t lax(uint16_t address);
size_t rla(uint16_t address);
size_t rra(uint16_t address);
size_t sax(uint16_t address);
size_t slo(uint16_t address);
size_t sre(uint16_t address);
size_t shy(uint16_t address);
size_t shx(uint16_t address);
size_t tas(uint16_t address);
size_t xaa(uint16_t address);

#endif

