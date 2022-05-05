#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#define HEADER_SIZE 16
#define TRAINER_SIZE 512

uint8_t* read_rom(char* file_name);
nes_header parse_header(uint8_t* rom);
nes_mapper init_mapper(uint8_t* rom, nes_header header);
uint16_t get_reset_vector(nes_bus* bus);
uint16_t get_irq_vector(nes_bus* bus);
uint16_t get_nmi_vector(nes_bus* bus);
uint8_t mapper_read(nes_bus* bus, uint16_t address);
void mapper_write(nes_bus* bus, uint8_t value, uint16_t address);

#endif