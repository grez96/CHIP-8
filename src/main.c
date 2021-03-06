#include <stdio.h>

#include "Chip8/Chip8.h"
#include "graphics/GFXscreen.h"


void clear_screen(void);
void print_menu(void);
const char *parse_num_to_program(unsigned num);

void run_emulator(const char *program);
void default_keypad_keyboard_mapping(GFXscreen gfxs);


int main(void)
{
	unsigned input;
	for (;;) {
		print_menu();
		
		if (scanf("%d", &input) != 1)
			continue;
		if (input > 7)
			continue;
		if (input == 0)
			break;

		run_emulator(parse_num_to_program(input));
        clear_screen();
	}

	return 0;
}

void clear_screen(void)
{
	for (int i = 0; i < 100; ++i)
		putchar('\n');
}

void print_menu(void)
{
	printf("----------------------------\n");
	printf("CHIP-8\n");
	printf("----------------------------\n");
	printf("[1] breakout\n");
	printf("[2] cave\n");
	printf("[3] coin flipping\n");
	printf("[4] pong\n");
	printf("[5] russian roulette\n");
	printf("[6] soccer\n");
	printf("[7] tank\n");
	
	printf("\n[0] EXIT\n");
	printf("----------------------------\n");
}

const char *parse_num_to_program(unsigned num)
{
	char *program;
	switch (num) {
	case 1:
		program = "../programs/breakout.ch8";
		break;
	case 2:
		program = "../programs/cave.ch8";
		break;
	case 3:
		program = "../programs/coin_flipping.ch8";
		break;
	case 4:
		program = "../programs/pong.ch8";
		break;
	case 5:
		program = "../programs/russian_roulette.ch8";
		break;
	case 6:
		program = "../programs/soccer.ch8";
		break;
	case 7:
		program = "../programs/tank.ch8";
		break;
	}
	return program;
}

void run_emulator(const char *program)
{
	Chip8 c8 = chip8_create();
	chip8_load_program(c8, program);

	GFXscreen gfxs = GFXscreen_create(1200, 800, "CHIP-8", CHIP8_DISPLAY_WIDTH,
		CHIP8_DISPLAY_HEIGHT, 0xFFFFFF, 0x000000, 500, 10);
	default_keypad_keyboard_mapping(gfxs);

	while (!GFXscreen_window_close(gfxs)) {
		GFXscreen_process_input(gfxs);
		chip8_execute_opcode(c8, GFXscreen_get_keypad_state_map(gfxs));
		GFXscreen_draw_frame(gfxs, chip8_get_gfx(c8));
	}

	GFXscreen_destroy(gfxs);
	chip8_destroy(c8);
}

void default_keypad_keyboard_mapping(GFXscreen gfxs)
{
    printf("default keybindings\n");
    printf("Keypad		Keyboard\n");
    printf("+-+-+-+-+	+-+-+-+-+\n");
    printf("|1|2|3|C|	|1|2|3|4|\n");
    printf("+-+-+-+-+	+-+-+-+-+\n");
    printf("|4|5|6|D|	|Q|W|E|R|\n");
    printf("+-+-+-+-+	+-+-+-+-+\n");
    printf("|7|8|9|E|	|A|S|D|F|\n");
    printf("+-+-+-+-+	+-+-+-+-+\n");
    printf("|A|0|B|F|	|Z|X|C|V|\n");
    printf("+-+-+-+-+	+-+-+-+-+\n");

	GFXscreen_map_keypad_keyboard(gfxs, 0, 'X');
	GFXscreen_map_keypad_keyboard(gfxs, 1, '1');
	GFXscreen_map_keypad_keyboard(gfxs, 2, '2');
	GFXscreen_map_keypad_keyboard(gfxs, 3, '3');
	GFXscreen_map_keypad_keyboard(gfxs, 4, 'Q');
	GFXscreen_map_keypad_keyboard(gfxs, 5, 'W');
	GFXscreen_map_keypad_keyboard(gfxs, 6, 'E');
	GFXscreen_map_keypad_keyboard(gfxs, 7, 'A');
	GFXscreen_map_keypad_keyboard(gfxs, 8, 'S');
	GFXscreen_map_keypad_keyboard(gfxs, 9, 'D');
	GFXscreen_map_keypad_keyboard(gfxs, 10, 'Z');
	GFXscreen_map_keypad_keyboard(gfxs, 11, 'C');
	GFXscreen_map_keypad_keyboard(gfxs, 12, '4');
	GFXscreen_map_keypad_keyboard(gfxs, 13, 'R');
	GFXscreen_map_keypad_keyboard(gfxs, 14, 'F');
	GFXscreen_map_keypad_keyboard(gfxs, 15, 'V');
}
