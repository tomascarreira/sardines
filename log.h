#ifndef LOG_H
#define LOG_H

extern size_t cycles;

void log_instr(nes_cpu cpu);
uint8_t log_read(uint16_t address);

void log_imp(nes_cpu cpu);
void log_imm(nes_cpu cpu);
void log_zp(nes_cpu cpu);
void log_zpx(nes_cpu cpu);
void log_zpy(nes_cpu cpu);
void log_abl(nes_cpu cpu);
void log_abx(nes_cpu cpu);
void log_aby(nes_cpu cpu);
void log_rel(nes_cpu cpu);
void log_ind(nes_cpu cpu);
void log_izx(nes_cpu cpu);
void log_izy(nes_cpu cpu);

#endif