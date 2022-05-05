#include "common.h"
#include "cpu.h"
#include "log.h"

char* log_opcode_table[256] = {
	"BRK", "ORA", "KIL", "SLO", "NOP", "ORA", "ASL", "SLO", "PHP", "ORA", "ASL", "ANC", "NOP", "ORA", "ASL", "SLO",
	"BPL", "ORA", "KIL", "SLO", "NOP", "ORA", "ASL", "SLO", "CLC", "ORA", "NOP", "SLO", "NOP", "ORA", "ASL", "SLO",
	"JSR", "AND", "KIL", "RLA", "BIT", "AND", "ROL", "RLA", "PLP", "AND", "ROL", "ANC", "BIT", "AND", "ROL", "RLA",
	"BMI", "AND", "KIL", "RLA", "NOP", "AND", "ROL", "RLA", "SEC", "AND", "NOP", "RLA", "NOP", "AND", "ROL", "RLA",
	"RTI", "EOR", "KIL", "SRE", "NOP", "EOR", "LSR", "SRE", "PHA", "EOR", "LSR", "ALR", "JMP", "EOR", "LSR", "SRE",
	"BVC", "EOR", "KIL", "SRE", "NOP", "EOR", "LSR", "SRE", "CLI", "EOR", "NOP", "SRE", "NOP", "EOR", "LSR", "SRE",
	"RTS", "ADC", "KIL", "RRA", "NOP", "ADC", "ROR", "RRA", "PLA", "ADC", "ROR", "ARR", "JMP", "ADC", "ROR", "RRA",
	"BVS", "ADC", "KIL", "RRA", "NOP", "ADC", "ROR", "RRA", "SEI", "ADC", "NOP", "RRA", "NOP", "ADC", "ROR", "RRA",
	"NOP", "STA", "NOP", "SAX", "STY", "STA", "STX", "SAX", "DEY", "NOP", "TXA", "XAA", "STY", "STA", "STX", "SAX",
	"BCC", "STA", "KIL", "AHX", "STY", "STA", "STX", "SAX", "TYA", "STA", "TXS", "TAS", "SHY", "STA", "SHX", "AHX",
	"LDY", "LDA", "LDX", "LAX", "LDY", "LDA", "LDX", "LAX", "TAY", "LDA", "TAX", "LAX", "LDY", "LDA", "LDX", "LAX",
	"BCS", "LDA", "KIL", "LAX", "LDY", "LDA", "LDX", "LAX", "CLV", "LDA", "TSX", "LAS", "LDY", "LDA", "LDX", "LAX",
	"CPY", "CMP", "NOP", "DCP", "CPY", "CMP", "DEC", "DCP", "INY", "CMP", "DEX", "AXS", "CPY", "CMP", "DEC", "DCP",
	"BNE", "CMP", "KIL", "DCP", "NOP", "CMP", "DEC", "DCP", "CLD", "CMP", "NOP", "DCP", "NOP", "CMP", "DEC", "DCP",
	"CPX", "SBC", "NOP", "ISC", "CPX", "SBC", "INC", "ISC", "INX", "SBC", "NOP", "SBC", "CPX", "SBC", "INC", "ISC",
	"BEQ", "SBC", "KIL", "ISC", "NOP", "SBC", "INC", "ISC", "SED", "SBC", "NOP", "SBC", "INC", "ISC"
};

const size_t log_opcode_size_table[256] = {
	1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
	2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
	3, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
	2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
	1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
	2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
	1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
	2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
	2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
	2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
	2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
	2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3
};

void (*log_addressing_mode_table[256])(nes_bus*, nes_cpu*) = {
		log_imp, log_izx, log_imp, log_izx,  log_zp,  log_zp,  log_zp,  log_zp, log_imp, log_imm, log_imp, log_imm, log_abl, log_abl, log_abl, log_abl,
		log_rel, log_izy, log_imp, log_izy, log_zpx, log_zpx, log_zpx, log_zpx, log_imp, log_aby, log_imp, log_aby, log_abx, log_abx, log_abx, log_abx,
		log_abl, log_izx, log_imp, log_izx,  log_zp,  log_zp,  log_zp,  log_zp, log_imp, log_imm, log_imp, log_imm, log_abl, log_abl, log_abl, log_abl,
		log_rel, log_izy, log_imp, log_izy, log_zpx, log_zpx, log_zpx, log_zpx, log_imp, log_aby, log_imp, log_aby, log_abx, log_abx, log_abx, log_abx,
		log_imp, log_izx, log_imp, log_izx,  log_zp,  log_zp,  log_zp,  log_zp, log_imp, log_imm, log_imp, log_imm, log_abl, log_abl, log_abl, log_abl,
		log_rel, log_izy, log_imp, log_izy, log_zpx, log_zpx, log_zpx, log_zpx, log_imp, log_aby, log_imp, log_aby, log_abx, log_abx, log_abx, log_abx,
		log_imp, log_izx, log_imp, log_izx,  log_zp,  log_zp,  log_zp,  log_zp, log_imp, log_imm, log_imp, log_imm, log_ind, log_abl, log_abl, log_abl,
		log_rel, log_izy, log_imp, log_izy, log_zpx, log_zpx, log_zpx, log_zpx, log_imp, log_aby, log_imp, log_aby, log_abx, log_abx, log_abx, log_abx,
		log_imm, log_izx, log_imm, log_izx,  log_zp,  log_zp,  log_zp,  log_zp, log_imp, log_imm, log_imp, log_imm, log_abl, log_abl, log_abl, log_abl,
		log_rel, log_izy, log_imp, log_izy, log_zpx, log_zpx, log_zpy, log_zpy, log_imp, log_aby, log_imp, log_aby, log_abx, log_abx, log_aby, log_aby,
		log_imm, log_izx, log_imm, log_izx,  log_zp,  log_zp,  log_zp,  log_zp, log_imp, log_imm, log_imp, log_imm, log_abl, log_abl, log_abl, log_abl,
		log_rel, log_izy, log_imp, log_izy, log_zpx, log_zpx, log_zpy, log_zpy, log_imp, log_aby, log_imp, log_aby, log_abx, log_abx, log_aby, log_aby,
		log_imm, log_izx, log_imm, log_izx,  log_zp,  log_zp,  log_zp,  log_zp, log_imp, log_imm, log_imp, log_imm, log_abl, log_abl, log_abl, log_abl,
		log_rel, log_izy, log_imp, log_izy, log_zpx, log_zpx, log_zpx, log_zpx, log_imp, log_aby, log_imp, log_aby, log_abx, log_abx, log_abx, log_abx,
		log_imm, log_izx, log_imm, log_izx,  log_zp,  log_zp,  log_zp,  log_zp, log_imp, log_imm, log_imp, log_imm, log_abl, log_abl, log_abl, log_abl,
		log_rel, log_izy, log_imp, log_izy, log_zpx, log_zpx, log_zpx, log_zpx, log_imp, log_aby, log_imp, log_aby, log_abx, log_abx, log_abx, log_abx
};

void log_instr(nes_bus* bus, nes_cpu* cpu) {

	uint8_t opcode = log_read(bus, cpu->pc);
	
	char* opcode_str = log_opcode_table[opcode];
	size_t opcode_size = log_opcode_size_table[opcode];

	printf("%04X  %02X", cpu->pc, opcode);
	switch (opcode_size) {
		case 1:
				printf("        ");
				break;

		case 2: 
				printf(" %02X     ", log_read(bus, cpu->pc + 1));
				break;

		case 3: 
				printf(" %02X %02X  ", log_read(bus, cpu->pc + 1), 
						log_read(bus, cpu->pc + 2));
				break;
	}
	printf("%s ", opcode_str);
	log_addressing_mode_table[opcode](bus, cpu);
	printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X ", cpu->a, cpu->x, cpu->y, colapse_status(cpu), cpu->s);
	printf("CYC:%lu", cycles);
	printf("\n");
}

uint8_t log_read(nes_bus* bus, uint16_t address) {

	uint8_t value;

	if (address <= 0x1fff) {
		value = bus->ram[address & 0x7ff];

	} else if (address >= 0x2000 && address <= 0x3fff) {
		value = 0;

	} else if (address >= 0x4000 && address <= 0x401f){
		value = 0;

	} else if (address >= 0x4020 && address <= 0xffff) {
	
		switch (bus->mapper.header.number) {

			case 0:
				if (address >= 0x6000 && address <= 0x7fff) {
					value = bus->mapper.prgram[address - 0x6000];

				} else if (address >= 0x8000 && address <= 0xbfff) {
					value = bus->mapper.rom[address - 0x8000];

				} else if (address >= 0xc000 && address <= 0xffff) {
					if (bus->mapper.header.prgrom == 1) {
						value = bus->mapper.rom[address - 0xc000];
					} else if (bus->mapper.header.prgrom == 2) {
						value = bus->mapper.rom[address - 0x8000];
					} else {
						printf("Bad mapper!\n");
						exit(EXIT_FAILURE);
					}

				} else {
					printf("Read mapper at %02x not implemented.\n", address);
					exit(EXIT_FAILURE);
				}

				break;

			default:
				printf("Mapper number %03d not supported.\n", bus->mapper.header.number);
				exit(EXIT_FAILURE);
		}
	}

	return value;
}

void log_imp(nes_bus* bus, nes_cpu* cpu) {

	uint8_t opcode = log_read(bus, cpu->pc);

	if (opcode == 0x0a || opcode == 0x4a || opcode == 0x2a || opcode == 0x6a) {
		printf("A                           ");

	} else {
		printf("                            ");
	}
}

void log_imm(nes_bus* bus, nes_cpu* cpu) {
	
	printf("#$%02X                        ", log_read(bus, cpu->pc + 1));
	
}

void log_zp(nes_bus* bus, nes_cpu* cpu) {

	uint8_t tmp = log_read(bus, cpu->pc + 1);

	printf("$%02X = %02X                    ", tmp, log_read(bus, tmp));
}

void log_zpx(nes_bus* bus, nes_cpu* cpu) {

	uint8_t tmp = log_read(bus, cpu->pc + 1);

	printf("$%02X = %02X                    ",
			tmp, log_read(bus, (tmp + cpu->x) & 0xff));	
}

void log_zpy(nes_bus* bus, nes_cpu* cpu) {

	uint8_t tmp = log_read(bus, cpu->pc + 1);

	printf( "$%02X = %02X                    ",
			tmp, log_read(bus, (tmp + cpu->y) & 0xff));	
}

void log_abl(nes_bus* bus, nes_cpu* cpu) {

	uint8_t hi = log_read(bus, cpu->pc + 2);
	uint8_t lo = log_read(bus, cpu->pc + 1);
	uint16_t tmp = (hi << 8) | lo;

	uint8_t opcode = log_read(bus, cpu->pc);

	if (opcode == 0x4c || opcode == 0x20) {
		printf("$%04X                       ", tmp);

	} else {
		printf("$%04X = %02X                  ", tmp, log_read(bus, tmp));
	}
		
}

void log_abx(nes_bus* bus, nes_cpu* cpu) {

	uint8_t hi = log_read(bus, cpu->pc + 1);
	uint8_t lo = log_read(bus, cpu->pc + 2);
	uint16_t tmp = ((hi << 8) | lo) + cpu->x; 

	printf("$%02X%02X = %02X                  ", hi, lo, log_read(bus, tmp));	
}

void log_aby(nes_bus* bus, nes_cpu* cpu) {

	uint8_t hi = log_read(bus, cpu->pc + 1);
	uint8_t lo = log_read(bus, cpu->pc + 2);
	uint16_t tmp = ((hi << 8) | lo) + cpu->y; 

	printf("$%02X%02X = %02X                  ", hi, lo, log_read(bus, tmp));	
}

void log_rel(nes_bus* bus, nes_cpu* cpu) {

	int8_t offset = log_read(bus, cpu->pc + 1);

	printf("$%04X                       ", cpu->pc + offset + 2);	
}

void log_ind(nes_bus* bus, nes_cpu* cpu) {

	uint8_t hi = log_read(bus, cpu->pc + 1);
	uint8_t lo = log_read(bus, cpu->pc + 2);
	uint16_t tmp = (hi << 8) | lo;

	uint16_t addr;
	if (lo == 0xff) {
		addr = (log_read(bus, tmp & 0xff) << 8) | (log_read(bus, tmp));

	} else {
		addr = (log_read(bus, tmp + 1) << 8) | (log_read(bus, tmp));
	}

	printf("($%04X) = %04X              ", tmp, log_read(bus, addr));	
}

void log_izx(nes_bus* bus, nes_cpu* cpu) {
	
	uint8_t zp_ptr = log_read(bus, cpu->pc + 1);

	uint8_t hi = log_read(bus, (zp_ptr + cpu->x + 1) & 0xff);
	uint8_t lo = log_read(bus, (zp_ptr + cpu->x) & 0xff);
	uint16_t tmp = (hi << 8) | lo;

	printf("($%02X,X) @ %02X = %04X = %02X    ",
			zp_ptr, (zp_ptr + cpu->x) & 0xff, tmp, log_read(bus, tmp));	
}

void log_izy(nes_bus* bus, nes_cpu* cpu) {

	uint8_t zp_ptr = cpu->pc + 1;

	uint8_t lo = log_read(bus, zp_ptr);
	uint8_t hi = log_read(bus, (zp_ptr + 1) & 0xff);
	uint16_t tmp = ((hi << 8) | lo) + cpu->y;

	printf("($%02X),Y = %04X @ %04X = %02X  ",
		zp_ptr, (hi << 8) | lo, tmp, log_read(bus, tmp));
}