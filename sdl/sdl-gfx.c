
#include "sdl-gfx.h"

int sdl_window_init(SDLWindow *window) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    window->window = SDL_CreateWindow(SDL_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOW_WIDTH,
                                      SDL_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window->window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    window->surface = SDL_GetWindowSurface(window->window);

    window->title = SDL_WINDOW_TITLE;
    window->width = SDL_WINDOW_WIDTH;
    window->height = SDL_WINDOW_HEIGHT;

    SDL_UpdateWindowSurface(window->window);

    return 0;
}

void sdl_event_loop(SDLWindow *window) {
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
        SDL_UpdateWindowSurface(window->window);

        // Add a small delay to avoid overloading the CPU
        SDL_Delay(16); // Roughly 60 FPS
    }
}

void sdl_window_destroy(SDLWindow *window) {
    SDL_DestroyWindow(window->window);
    SDL_Quit();
}