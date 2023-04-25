#include "input.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t controller_1_data = 0;
static uint8_t controller_2_data = 0;
static uint8_t tmp_controller_1_data = 0;
static uint8_t tmp_controller_2_data = 0;
static bool is_polling = false;

// Fix: On the nes controller after the 8 bits are shift it returns 1
uint8_t get_controller_data(size_t contr_id) {
	uint8_t res = 0;
	if (contr_id == 1) {
		res = controller_1_data & 0x1;
		controller_1_data >>= 1;
	} else if (contr_id == 2) {
		res = controller_2_data & 0x1;
		controller_2_data >>= 1;
	}

	return res;
}

void set_controller_data(size_t contr_id, enum contr_buttons button_id, bool button_pressed) {
	if (contr_id == 1) {
		tmp_controller_1_data = (tmp_controller_1_data & ~(0x1 << button_id)) | (button_pressed << button_id);
		} else if (contr_id == 2) {
		tmp_controller_2_data = (tmp_controller_2_data & ~(0x1 << button_id)) | (button_pressed << button_id);	
	}
}

void update_input(void) {
	if (is_polling) {
		controller_1_data = tmp_controller_1_data;
		controller_2_data = tmp_controller_2_data;
	}
}

void poll_controller(void) {
	is_polling = true;
}

void end_poll_controller(void) {
	is_polling = false;
}
