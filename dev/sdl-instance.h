#ifndef SDL_INSTANCE_H
#define SDL_INSTANCE_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

// SDL window properties
#define SDL_WINDOW_WIDTH 1174
#define SDL_WINDOW_HEIGHT 772
#define SDL_WINDOW_TITLE "NES Emulator"

#define WINDOW_QUIT (1 << 8)

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
 * sdl_put_pixel() can be used to set the value of a single pixel in
 * pixel_buffer. sdl_draw_frame() renders the pixel_buffer to the window using
 * the GPU. sdl_poll_events() checks if the user has pressed a key or requested
 * to quit the window. sdl_instance_destroy() needs to be called when quitting
 * the window, to avoid memory leaks.
 */
int sdl_instance_init();
void sdl_clear_screen();
void sdl_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void sdl_draw_frame();
uint32_t sdl_poll_events();
void sdl_instance_destroy();

/*
 * This struct represents a square region within the entire SDL window.
 * The idea is to have one WindowRegion represent the NES SCREEN and one
 * DEBUG screen:
 *
 *      |--SDL WINDOW------------------------------------------------|
 *      |                                                            |
 *      |    |--NES SCREEN---------------|   |--DEBUG SCREEN-----|   |
 *      |    |                           |   |                   |   |
 *      |    |                           |   |                   |   |
 *      |    |                           |   |                   |   |
 *      |    |                           |   |                   |   |
 *      |    |                           |   |                   |   |
 *      |    |                           |   |                   |   |
 *      |    |                           |   |                   |   |
 *      |    |                           |   |                   |   |
 *      |    |---------------------------|   |-------------------|   |
 *      |                                                            |
 *      |------------------------------------------------------------|
 */
typedef struct WindowRegion {
    const uint32_t top_coord;
    const uint32_t left_coord;
    const uint32_t width;
    const uint32_t height;
    const uint32_t scale_factor;
} WindowRegion;

/*
 * This function puts a pixel relative to the top left of the WindowRegion.
 * It does this by calculating where that pixel should go in the pixel_buffer of the SDLInstance
 *
 * The size and width of the pixel will be equal to window_region->scale_factor
 * This is because the actual resolution of the NES is very tiny by today's standards
 */
void sdl_put_pixel_region(WindowRegion *window_region, int relative_x, int relative_y, uint32_t color);


#endif
