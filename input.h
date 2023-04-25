#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum contr_buttons {
	a,
	b,
	contr_select,
	contr_start,
	up,
	down,
	left,
	right,
};

uint8_t get_controller_data(size_t contr_id);
void set_controller_data(size_t contr_id, enum contr_buttons button_id, bool button_pressed);
void update_input(void);
void poll_controller(void);
void end_poll_controller(void);
#endif
