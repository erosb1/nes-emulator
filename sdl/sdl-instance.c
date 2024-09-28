
#include "sdl-instance.h"

int sdl_instance_init(SDLInstance *sdl_instance) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    sdl_instance->window = SDL_CreateWindow(SDL_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOW_WIDTH,
                                      SDL_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (sdl_instance->window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    sdl_instance->renderer = SDL_CreateRenderer(sdl_instance->window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl_instance->renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdl_instance->window);
        return -1;
    }

    sdl_instance->texture = SDL_CreateTexture(sdl_instance->renderer, SDL_PIXELFORMAT_ARGB8888,
                            SDL_TEXTUREACCESS_STREAMING, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT);

    sdl_instance->title = SDL_WINDOW_TITLE;
    sdl_instance->pixel_buffer = (uint32_t *)malloc(SDL_WINDOW_WIDTH * SDL_WINDOW_HEIGHT * sizeof(Uint32));
    sdl_instance->width = SDL_WINDOW_WIDTH;
    sdl_instance->height = SDL_WINDOW_HEIGHT;

    return 0;
}

void sdl_clear_screen(SDLInstance *sdl_instance) {
    for (int i = 0; i < sdl_instance->width * sdl_instance->height; i++) {
        sdl_instance->pixel_buffer[i] = 0x00000000;
    }
}

void sdl_put_pixel(SDLInstance *sdl_instance, int x, int y, uint32_t color) {
    if (x >= 0 && x < sdl_instance->width && y >= 0 && y < sdl_instance->height) {
        sdl_instance->pixel_buffer[y * sdl_instance->width + x] = color;
    }
}

void sdl_draw_frame(SDLInstance *sdl_instance) {
    SDL_UpdateTexture(sdl_instance->texture, NULL, sdl_instance->pixel_buffer, sdl_instance->width * sizeof(Uint32));
    SDL_RenderClear(sdl_instance->renderer);
    SDL_RenderCopy(sdl_instance->renderer, sdl_instance->texture, NULL, NULL);
    SDL_RenderPresent(sdl_instance->renderer);
}

uint32_t sdl_poll_events() {
    SDL_Event event;
    uint32_t event_mask = NONE;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            event_mask |= WINDOW_QUIT;
        }
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_X]) {
            event_mask |= NES_A_BUTTON;
        }
        if (state[SDL_SCANCODE_Z]) {
            event_mask |= NES_B_BUTTON;
        }
        if (state[SDL_SCANCODE_RSHIFT]) {
            event_mask |= NES_SELECT_BUTTON;
        }
        if (state[SDL_SCANCODE_RETURN]) {
            event_mask |= NES_START_BUTTON;
        }
        if (state[SDL_SCANCODE_UP]) {
            event_mask |= NES_DPAD_UP;
        }
        if (state[SDL_SCANCODE_DOWN]) {
            event_mask |= NES_DPAD_DOWN;
        }
        if (state[SDL_SCANCODE_LEFT]) {
            event_mask |= NES_DPAD_LEFT;
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            event_mask |= NES_DPAD_RIGHT;
        }
    }

    return event_mask;
}

void sdl_instance_destroy(SDLInstance *sdl_instance) {
    if (sdl_instance->pixel_buffer) free(sdl_instance->pixel_buffer);
    if (sdl_instance->texture) SDL_DestroyTexture(sdl_instance->texture);
    if (sdl_instance->renderer) SDL_DestroyRenderer(sdl_instance->renderer);
    if (sdl_instance->window) SDL_DestroyWindow(sdl_instance->window);
    SDL_Quit();
}