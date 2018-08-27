#ifndef CHIP8_H
#define CHIP8_H


#include "../utility/utility.h"


#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_DISPLAY_HEIGHT 32


typedef struct Chip8_t* Chip8;


Chip8 chip8_create(void);

void chip8_load_program(Chip8 c8, const char *file_path);

const unsigned char* chip8_get_gfx(const Chip8 c8);

void chip8_execute_opcode(Chip8 c8, const Map keypad_state_map);

void chip8_destroy(Chip8 c8);


#endif
