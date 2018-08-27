#include "Chip8/Chip8.h"
#include "graphics/GFXscreen.h"


void default_keypad_keyboard_mapping(GFXscreen gfxs);


int main(void)
{
	Chip8 c8 = chip8_create();
	chip8_load_program(c8, "../programs/pong.ch8");

	GFXscreen gfxs = GFXscreen_create(1200, 800, "CHIP-8", CHIP8_DISPLAY_WIDTH,
		CHIP8_DISPLAY_HEIGHT, 0xFFFFFF, 0x000000, 500, 10);
	default_keypad_keyboard_mapping(gfxs);

	while(!GFXscreen_window_close(gfxs)) {
		GFXscreen_process_input(gfxs);
		chip8_execute_opcode(c8, GFXscreen_get_keypad_state_map(gfxs));
		GFXscreen_draw_frame(gfxs, chip8_get_gfx(c8));
	}

	GFXscreen_destroy(gfxs);
	chip8_destroy(c8);
	return 0;
}

/*
 * Keypad		Keyboard
 * +-+-+-+-+	+-+-+-+-+
 * |1|2|3|C|	|1|2|3|4|
 * +-+-+-+-+	+-+-+-+-+
 * |4|5|6|D|	|Q|W|E|R|
 * +-+-+-+-+	+-+-+-+-+
 * |7|8|9|E|	|A|S|D|F|
 * +-+-+-+-+	+-+-+-+-+
 * |A|0|B|F|	|Z|X|C|V|
 * +-+-+-+-+	+-+-+-+-+
 */
void default_keypad_keyboard_mapping(GFXscreen gfxs)
{
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
