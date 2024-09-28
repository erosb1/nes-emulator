
#include "sdl-instance.h"

int sdl_instance_init(SDLInstance *instance) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    instance->window = SDL_CreateWindow(SDL_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOW_WIDTH,
                                      SDL_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (instance->window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    instance->surface = SDL_GetWindowSurface(instance->window);

    instance->title = SDL_WINDOW_TITLE;
    instance->width = SDL_WINDOW_WIDTH;
    instance->height = SDL_WINDOW_HEIGHT;

    SDL_UpdateWindowSurface(instance->window);

    return 0;
}

void sdl_event_loop(SDLInstance *instance) {
    int quit = 0;
    SDL_Event e;

    // Event loop
    while (!quit) {
        // Handle events on the queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        // Optionally, you can clear and update the surface in each loop iteration
        SDL_UpdateWindowSurface(instance->window);

        // Add a small delay to avoid overloading the CPU
        SDL_Delay(16); // Roughly 60 FPS
    }
}

void sdl_instance_destroy(SDLInstance *instance) {
    SDL_DestroyWindow(instance->window);
    SDL_Quit();
}