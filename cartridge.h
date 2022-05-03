#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#define HEADER_SIZE 16
#define TRAINER_SIZE 512

struct mapper000_registers;

uint8_t* read_rom(char* file_name);
nes_header parse_header(uint8_t* rom);
mapper init_mapper(uint8_t* rom, nes_header header);
uint16_t get_reset_vector(bus* bus);

#endif