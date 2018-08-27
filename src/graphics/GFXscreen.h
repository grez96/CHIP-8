#ifndef GRAPHICS_GFX_SCREEN_H
#define GRAPHICS_GFX_SCREEN_H


#include <stdbool.h>

#include "../utility/utility.h"


typedef struct GFXscreen_t* GFXscreen;


GFXscreen GFXscreen_create(unsigned w, unsigned h, const char *title,
	unsigned gfx_w, unsigned gfx_h, long color_on, long color_off,
	unsigned fps, unsigned boarder_thickns);

bool GFXscreen_window_close(GFXscreen gfxs);

void GFXscreen_map_keypad_keyboard(GFXscreen gfxs, unsigned keypad,
	char keyboard);

void GFXscreen_process_input(GFXscreen gfxs);

const Map GFXscreen_get_keypad_state_map(GFXscreen gfxs);

void GFXscreen_draw_frame(GFXscreen gfxs, const unsigned char gfx[]);

void GFXscreen_destroy(GFXscreen gfxs);


#endif
