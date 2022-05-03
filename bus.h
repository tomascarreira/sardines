#ifndef BUS_H
#define BUS_H

typedef struct bus bus;
struct bus {
	uint8_t* ram;
	mapper mapper;
};

bus* init_bus(mapper mapper); 

#endif