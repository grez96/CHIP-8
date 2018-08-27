#include "Chip8.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define FNAME "Chip8.c"

#define MEMORY_SZ 0xFFF
#define V_SZ 0x10
#define GFX_SZ CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT
#define STACK_SZ 0x10


static void initialize_chip8(Chip8 c8);
static void load_fontset(Chip8 c8);

static void update_timers(Chip8 c8, unsigned hz);

static void opcode_00e0(Chip8 c8);
static void opcode_00ee(Chip8 c8);
static void opcode_1nnn(Chip8 c8);
static void opcode_2nnn(Chip8 c8);
static void opcode_3xnn(Chip8 c8);
static void opcode_4xkk(Chip8 c8);
static void opcode_5xy0(Chip8 c8);
static void opcode_6xkk(Chip8 c8);
static void opcode_7xnn(Chip8 c8);
static void opcode_8xy0(Chip8 c8);
static void opcode_8xy2(Chip8 c8);
static void opcode_8xy4(Chip8 c8);
static void opcode_8xy5(Chip8 c8);
static void opcode_annn(Chip8 c8);
static void opcode_cxnn(Chip8 c8);
static void opcode_dxyn(Chip8 c8);
static void opcode_ex9e(Chip8 c8, Map keypad_state_map);
static void opcode_exa1(Chip8 c8, Map keypad_state_map);
static void opcode_fx07(Chip8 c8);
static void opcode_fx0a(Chip8 c8, Map keypad_state_map);
static void opcode_fx15(Chip8 c8);
static void opcode_fx18(Chip8 c8);
static void opcode_fx1e(Chip8 c8);
static void opcode_fx29(Chip8 c8);
static void opcode_fx33(Chip8 c8);
static void opcode_fx55(Chip8 c8);
static void opcode_fx65(Chip8 c8);


static bool instance_exists;


struct Chip8_t {
	unsigned short opcode;
	unsigned char memory[MEMORY_SZ];
	unsigned char V[V_SZ];
	unsigned short I;
	unsigned short pc;
	unsigned char gfx[GFX_SZ];
	unsigned char delay_timer;
	unsigned char sound_timer;
	float prev_tick_time;
	unsigned short stack[STACK_SZ];
	unsigned short sp;
	bool execution_blocked;
};


Chip8 chip8_create(void)
{
	if (instance_exists)
		exit_log(FNAME, 1,
			"Failed creating Chip8, only one instance permitted.");

	Chip8 c8 = (Chip8) malloc(sizeof(struct Chip8_t));
	if (!c8)
		exit_log(FNAME, 1, "Failed creating Chip8, memory allocation fail.");

	static bool rand_seeded;
	if (!rand_seeded) {
		srand(time(NULL));
		rand_seeded = true;
	}

	initialize_chip8(c8);
	instance_exists = true;
	return c8;
}

static void initialize_chip8(Chip8 c8)
{
	memset(c8, 0, sizeof(struct Chip8_t));
	c8->pc = 0x200;
	load_fontset(c8);
	c8->execution_blocked = false;
}

static void load_fontset(Chip8 c8)
{
	unsigned char font[] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	memcpy(c8->memory, font, sizeof(font));
}

void chip8_load_program(Chip8 c8, const char *file_path)
{
	initialize_chip8(c8);

	FILE *f = fopen(file_path, "rb");
	if (!f)
		exit_log(FNAME, 1, "Failed loading program, invalid file path.");
	fread(c8->memory + 0x200, 1, MEMORY_SZ - 0x200, f);

	fclose(f);
}

const unsigned char* chip8_get_gfx(const Chip8 c8)
{
	return c8->gfx;
}

void chip8_execute_opcode(Chip8 c8, const Map keypad_state_map)
{
	if (!c8->memory[0x200] && !c8->memory[0x201])
		exit_log(FNAME, 1, "Failed executing opcode, no program loaded.");

	if (c8->execution_blocked) {
		opcode_fx0a(c8, keypad_state_map);
		return;
	}

	update_timers(c8, 60);
	c8->opcode = c8->memory[c8->pc] << 8 | c8->memory[c8->pc + 1];
	switch (c8->opcode & 0xF000) {
	case 0x0000:
		switch (c8->opcode & 0x000F) {
		case 0x0000:
			opcode_00e0(c8);
			break;

		case 0x000E:
			opcode_00ee(c8);
			break;

		default:
			goto UNKNOWN_OPCODE;
		}
		break;

	case 0x1000:
		opcode_1nnn(c8);
		break;

	case 0x2000:
		opcode_2nnn(c8);
		break;

	case 0x3000:
		opcode_3xnn(c8);
		break;

	case 0x4000:
		opcode_4xkk(c8);
		break;

	case 0x5000:
		opcode_5xy0(c8);
		break;

	case 0x6000:
		opcode_6xkk(c8);
		break;

	case 0x7000:
		opcode_7xnn(c8);
		break;

	case 0x8000:
		switch (c8->opcode & 0x000F) {
		case 0x0000:
			opcode_8xy0(c8);
			break;

		case 0x0002:
			opcode_8xy2(c8);
			break;

		case 0x0004:
			opcode_8xy4(c8);
			break;

		case 0x0005:
			opcode_8xy5(c8);
			break;

		default:
			goto UNKNOWN_OPCODE;
		}
		break;

	case 0xA000:
		opcode_annn(c8);
		break;

	case 0xC000:
		opcode_cxnn(c8);
		break;

	case 0xD000:
		opcode_dxyn(c8);
		break;

	case 0xE000:
		switch (c8->opcode & 0x00FF) {
		case 0x009E:
			opcode_ex9e(c8, keypad_state_map);
			break;

		case 0x00A1:
			opcode_exa1(c8, keypad_state_map);
			break;

		default:
			goto UNKNOWN_OPCODE;
		}
		break;

	case 0xF000:
		switch (c8->opcode & 0x00FF) {
		case 0x0007:
			opcode_fx07(c8);
			break;

		case 0x000A:
			opcode_fx0a(c8, keypad_state_map);
			break;

		case 0x0015:
			opcode_fx15(c8);
			break;

		case 0x0018:
			opcode_fx18(c8);
			break;

		case 0x001E:
			opcode_fx1e(c8);
			break;

		case 0x0029:
			opcode_fx29(c8);
			break;

		case 0x0033:
			opcode_fx33(c8);
			break;

		case 0x0055:
			opcode_fx55(c8);
			break;

		case 0x0065:
			opcode_fx65(c8);
			break;

		default:
			goto UNKNOWN_OPCODE;
		}
		break;

	default: UNKNOWN_OPCODE:
		;char opc_str[32];
		sprintf(opc_str, "\topcode: 0x%x", c8->opcode);
		exit_log(FNAME, 2, "Failed executing opcode, unknown opcode.", opc_str);
	}
	c8->pc += 2;
}

static void update_timers(Chip8 c8, unsigned hz)
{
	if (c8->delay_timer > 0) {
        float tick_duration = 1000.0 / hz / 1000.0;
		float curr_tick_time = (float)clock() / CLOCKS_PER_SEC;
		if (curr_tick_time - c8->prev_tick_time >= tick_duration) {
			--c8->delay_timer;
		    c8->prev_tick_time = curr_tick_time;
		}
	}

	if (c8->sound_timer > 0) {
        float tick_duration = 1000.0 / hz / 1000.0;
		float curr_tick_time = (float)clock() / CLOCKS_PER_SEC;
		if (curr_tick_time - c8->prev_tick_time >= tick_duration) {
			--c8->sound_timer;
			c8->prev_tick_time = curr_tick_time;
		}
	}
}

// clear the gfx
static void opcode_00e0(Chip8 c8)
{
	memset(c8->gfx, 0, GFX_SZ);
}

// return from subroutine
static void opcode_00ee(Chip8 c8)
{
	c8->pc = c8->stack[--c8->sp];
}

// set pc to nnn
static void opcode_1nnn(Chip8 c8)
{
	c8->pc = c8->opcode & 0x0FFF;
	c8->pc -= 2;
}

// call subroutine at address nnn
static void opcode_2nnn(Chip8 c8)
{
	c8->stack[c8->sp++] = c8->pc;
	c8->pc = c8->opcode & 0x0FFF;
	c8->pc -= 2;
}

// skip next instruction if V[x] == nn
static void opcode_3xnn(Chip8 c8)
{
	if (c8->V[(c8->opcode & 0x0F00) >> 8] == (c8->opcode & 0x00FF))
		c8->pc += 2;
}

// skip next instruction if V[x] != kk
static void opcode_4xkk(Chip8 c8)
{
	if (c8->V[(c8->opcode & 0x0F00) >> 8] != (c8->opcode & 0x00FF))
		c8->pc += 2;
}

// skip next instruction if V[x] == V[y]
static void opcode_5xy0(Chip8 c8)
{
	if (c8->V[(c8->opcode & 0x0F00) >> 8] == c8->V[(c8->opcode & 0x00F0) >> 4])
		c8->pc += 2;
}

// set V[x] to the value kk
static void opcode_6xkk(Chip8 c8)
{
	c8->V[(c8->opcode & 0x0F00) >> 8] = c8->opcode & 0x00FF;
}

// add nn to V[x]
static void opcode_7xnn(Chip8 c8)
{
	c8->V[(c8->opcode & 0x0F00) >> 8] += c8->opcode & 0x00FF;
}

// set V[x] to V[y]
static void opcode_8xy0(Chip8 c8)
{
	c8->V[(c8->opcode & 0x0F00) >> 8] = c8->V[(c8->opcode & 0x00F0) >> 4];
}

// set V[x] to bitwise AND of V[x] and V[y]
static void opcode_8xy2(Chip8 c8)
{
	c8->V[(c8->opcode & 0x0F00) >> 8] &= c8->V[(c8->opcode & 0x00F0) >> 4];
}

// set V[x] to V[x] + V[y], set V[0xF] if result > 8 bits, store first 8 bits
static void opcode_8xy4(Chip8 c8)
{
	unsigned short result =
		c8->V[(c8->opcode & 0x0F00) >> 8] + c8->V[(c8->opcode & 0x00F0) >> 4];
	c8->V[(c8->opcode & 0x0F00) >> 8] = result & 0xFF;
	c8->V[0xF] = result > 0xFF;
}

// set V[x] to V[x] - V[y], set V[0xF] if V[x] > V[y]
static void opcode_8xy5(Chip8 c8)
{
	c8->V[0xf] =
		c8->V[(c8->opcode & 0x0F00) >> 8] > c8->V[(c8->opcode & 0x00F0) >> 4];
	c8->V[(c8->opcode & 0x0F00) >> 8] -= c8->V[(c8->opcode & 0x00F0) >> 4];
}

// set I to address nnn
static void opcode_annn(Chip8 c8)
{
	c8->I = 0x0FFF & c8->opcode;
}

// set V[x] to a random number(0-255) & nn
static void opcode_cxnn(Chip8 c8)
{
	c8->V[(c8->opcode & 0x0F00) >> 8] = rand() % 0xFF & c8->opcode & 0x00FF;
}

// draw sprite from I, at (x,y), n pixels high, set V[0xF] on collision
static void opcode_dxyn(Chip8 c8)
{
	unsigned char x_pos = c8->V[(c8->opcode & 0x0F00) >> 8];
	unsigned char y_pos = c8->V[(c8->opcode & 0x00F0) >> 4];
	unsigned char height = c8->opcode & 0x000F;

	unsigned char pixel;
	for (unsigned char i = 0; i < height; ++i) {
		pixel = c8->memory[c8->I + i];
		for (unsigned char j = 0; j < 8; ++j) {
			if (pixel & 0x80 >> j) {
				c8->V[0xF] = c8->gfx[(y_pos + i) * 64 + x_pos + j];
				c8->gfx[(y_pos + i) * 64 + x_pos + j] ^= 1;
			}
		}
	}
}

// skip next instruction if key V[x] is pressed
static void opcode_ex9e(Chip8 c8, Map keypad_state_map)
{
	if (map_get(keypad_state_map, c8->V[(c8->opcode & 0x0F00) >> 8]))
		c8->pc += 2;
}

// skip next instruction if key V[x] is not pressed
static void opcode_exa1(Chip8 c8, Map keypad_state_map)
{
	if (!map_get(keypad_state_map, c8->V[(c8->opcode & 0x0F00) >> 8]))
		c8->pc += 2;
}

// set V[x] to delay timer
static void opcode_fx07(Chip8 c8)
{
	c8->V[(c8->opcode & 0x0F00) >> 8] = c8->delay_timer;
}

// halt execution until key is pressed, store key in V[x]
static void opcode_fx0a(Chip8 c8, Map keypad_state_map)
{
	for(size_t i = 0; i < map_get_size(keypad_state_map); ++i)
		if (map_get(keypad_state_map, i)) {
			c8->execution_blocked = false;
			c8->V[(c8->opcode & 0x0F00) >> 8] = i;
			return ;
		}

	c8->execution_blocked = true;
}

// set delay timer to V[x]
static void opcode_fx15(Chip8 c8)
{
	c8->delay_timer = c8->V[(c8->opcode & 0x0F00) >> 8];
	c8->prev_tick_time = (float)clock() / CLOCKS_PER_SEC;
}

// set sound timer to V[x]
static void opcode_fx18(Chip8 c8)
{
	c8->sound_timer = c8->V[(c8->opcode & 0x0F00) >> 8];
	c8->prev_tick_time = (float)clock() / CLOCKS_PER_SEC;
}

// set I = I + V[x]
static void opcode_fx1e(Chip8 c8)
{
	c8->I += c8->V[(c8->opcode & 0x0F00) >> 8];
}

// store sprite for character V[x] at I
static void opcode_fx29(Chip8 c8)
{
	c8->I = c8->V[(c8->opcode & 0x0F00) >> 8] * 5;
}

// store decimal value of V[x] starting at I
static void opcode_fx33(Chip8 c8)
{
	unsigned short num = c8->V[(c8->opcode & 0x0F00) >> 8];
	c8->memory[c8->I] = num / 100;
	c8->memory[c8->I + 1] = num % 100 / 10;
	c8->memory[c8->I + 2] = num  % 100 % 10;
}

// store V[0] - V[x] starting at I
static void opcode_fx55(Chip8 c8)
{
	for (size_t i = 0; i <= (c8->opcode & 0x0F00) >> 8; ++i)
		c8->memory[c8->I + i] = c8->V[i];
}

// fills V[0] - V[x] with values starting at I
static void opcode_fx65(Chip8 c8)
{
	for (unsigned char i = 0; i <= (c8->opcode & 0x0F00) >> 8; ++i)
		c8->V[i] = c8->memory[c8->I + i];
}

void chip8_destroy(Chip8 c8)
{
	free(c8);
	instance_exists = false;
}
