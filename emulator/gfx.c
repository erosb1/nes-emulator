#include "gfx.h"


#ifndef RISC_V
extern SDLInstance SDL_INSTANCE;
#endif

void put_pixel(uint32_t scanline, uint32_t dot, uint32_t color) {}

void draw_frame() {
#ifndef RISC_V
    sdl_draw_frame(&SDL_INSTANCE);
#endif
}