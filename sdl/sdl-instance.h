#ifndef SDL_INSTANCE_H
#define SDL_INSTANCE_H

#include <SDL2/SDL.h>

#define SDL_WINDOW_WIDTH  800
#define SDL_WINDOW_HEIGHT 600
#define SDL_WINDOW_TITLE  "NES Emulator"

typedef struct SDLInstance {
    SDL_Window *window;
    SDL_Surface *surface;
    int width;
    int height;
    const char *title;
} SDLInstance;

int sdl_instance_init(SDLInstance *instance);
void sdl_event_loop(SDLInstance *instance);
void sdl_instance_destroy(SDLInstance *instance);

#endif