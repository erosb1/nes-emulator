#ifndef SDL_GFX_H
#define SDL_GFX_H

#include <SDL2/SDL.h>

#define SDL_WINDOW_WIDTH  800
#define SDL_WINDOW_HEIGHT 600
#define SDL_WINDOW_TITLE  "NES Emulator"

typedef struct SDLWindow {
    SDL_Window *window;
    SDL_Surface *surface;
    int width;
    int height;
    const char *title;
} SDLWindow;

int sdl_window_init(SDLWindow *window);
void sdl_event_loop(SDLWindow *window);
void sdl_window_destroy(SDLWindow *window);

#endif