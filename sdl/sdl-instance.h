#ifndef SDL_INSTANCE_H
#define SDL_INSTANCE_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

#define SDL_WINDOW_WIDTH  800
#define SDL_WINDOW_HEIGHT 600
#define SDL_WINDOW_TITLE  "NES Emulator"

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

/*
 * sdl_instance_init() has to be called before any other function.
 *    It sets up the window, renderer, texture and pixel_buffer.
 * sdl_clear_screen() sets all values in pixel_buffer to 0x00000000.
 * sdl_put_pixel() can be used to set the value of a single pixel in pixel_buffer.
 * sdl_draw_frame() renders the pixel_buffer to the window using the GPU.
 * sdl_poll_events() checks if the user has pressed a key or requested to quit the window.
 * sdl_instance_destroy() needs to be called when quitting the window, to avoid memory leaks.
 */

int sdl_instance_init(SDLInstance *sdl_instance);
void sdl_clear_screen(SDLInstance *sdl_instance);
void sdl_put_pixel(SDLInstance *sdl_instance, int x, int y, uint32_t color);
void sdl_draw_frame(SDLInstance *sdl_instance);
uint32_t sdl_poll_events();
void sdl_instance_destroy(SDLInstance *sdl_instance);

#endif