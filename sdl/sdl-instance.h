#ifndef SDL_INSTANCE_H
#define SDL_INSTANCE_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

#define SDL_WINDOW_WIDTH  800
#define SDL_WINDOW_HEIGHT 600
#define SDL_WINDOW_TITLE  "NES Emulator"

// NES Controller to keyboard mapping
//#define NES_A_BUTTON      SDL_SCANCODE_X
//#define NES_B_BUTTON      SDL_SCANCODE_Z
//#define NES_SELECT_BUTTON SDL_SCANCODE_RSHIFT
//#define NES_START_BUTTON  SDL_SCANCODE_RETURN
//#define NES_DPAD_UP       SDL_SCANCODE_UP
//#define NES_DPAD_DOWN     SDL_SCANCODE_DOWN
//#define NES_DPAD_LEFT     SDL_SCANCODE_LEFT
//#define NES_DPAD_RIGHT    SDL_SCANCODE_RIGHT

typedef enum SDLEmulatorEvent {
    NONE              = 0,
    WINDOW_QUIT       = 1 << 0,
    NES_A_BUTTON      = 1 << 1,
    NES_B_BUTTON      = 1 << 2,
    NES_SELECT_BUTTON = 1 << 3,
    NES_START_BUTTON  = 1 << 4,
    NES_DPAD_UP       = 1 << 5,
    NES_DPAD_DOWN     = 1 << 6,
    NES_DPAD_LEFT     = 1 << 7,
    NES_DPAD_RIGHT    = 1 << 8,
} SDLEmulatorEvent;

typedef struct SDLInstance {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *pixel_buffer;
    int width;
    int height;
    const char *title;
} SDLInstance;

int sdl_instance_init(SDLInstance *sdl_instance);
void sdl_clear_screen(SDLInstance *sdl_instance);
void sdl_put_pixel(SDLInstance *sdl_instance, int x, int y, uint32_t color);
void sdl_draw_frame(SDLInstance *sdl_instance);
uint32_t sdl_poll_events(SDLInstance *sdl_instance);
void sdl_instance_destroy(SDLInstance *sdl_instance);

#endif