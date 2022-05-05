#ifndef LOG_H
#define LOG_H

extern size_t cycles;

void log_instr(nes_bus* bus, nes_cpu* cpu);
uint8_t log_read(nes_bus* bus, uint16_t address);

void log_imp(nes_bus* bus, nes_cpu* cpu);
void log_imm(nes_bus* bus, nes_cpu* cpu);
void log_zp(nes_bus* bus, nes_cpu* cpu);
void log_zpx(nes_bus* bus, nes_cpu* cpu);
void log_zpy(nes_bus* bus, nes_cpu* cpu);
void log_abl(nes_bus* bus, nes_cpu* cpu);
void log_abx(nes_bus* bus, nes_cpu* cpu);
void log_aby(nes_bus* bus, nes_cpu* cpu);
void log_rel(nes_bus* bus, nes_cpu* cpu);
void log_ind(nes_bus* bus, nes_cpu* cpu);
void log_izx(nes_bus* bus, nes_cpu* cpu);
void log_izy(nes_bus* bus, nes_cpu* cpu);

#endif