
#include "sdl-instance.h"
#include "emulator.h"
#include "common.h"

// Global SDLInstance variable
SDLInstance SDL_INSTANCE;

// Setup window regions
#define WINDOW_REGION_PADDING 50
#define NES_SCREEN_SCALE_FACTOR 3

#define DEBUG_SCREEN_SCALE_FACTOR 2
#define DEBUG_SCREEN_WIDTH 128
#define DEBUG_SCREEN_HEIGHT 336

WindowRegion NES_SCREEN = {
    .top_coord = 50,
    .left_coord = 50,
    .width = NES_SCREEN_WIDTH * NES_SCREEN_SCALE_FACTOR,
    .height = NES_SCREEN_HEIGHT * NES_SCREEN_SCALE_FACTOR,
    .scale_factor = NES_SCREEN_SCALE_FACTOR,
};

WindowRegion DEBUG_SCREEN = {
    .top_coord = 50,
    .left_coord = 868,
    .width = DEBUG_SCREEN_WIDTH * DEBUG_SCREEN_SCALE_FACTOR,
    .height = DEBUG_SCREEN_HEIGHT * DEBUG_SCREEN_SCALE_FACTOR,
    .scale_factor = DEBUG_SCREEN_SCALE_FACTOR,
};

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
    sdl_instance->pixel_buffer = (uint32_t *)malloc(SDL_WINDOW_WIDTH * SDL_WINDOW_HEIGHT * sizeof(uint32_t));
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
    SDL_UpdateTexture(sdl_instance->texture, NULL, sdl_instance->pixel_buffer, sdl_instance->width * sizeof(uint32_t));
    SDL_RenderClear(sdl_instance->renderer);
    SDL_RenderCopy(sdl_instance->renderer, sdl_instance->texture, NULL, NULL);
    SDL_RenderPresent(sdl_instance->renderer);
}

enum {
    NES_A_BUTTON = 1 << 0,
    NES_B_BUTTON = 1 << 1,
    NES_SELECT_BUTTON = 1 << 2,
    NES_START_BUTTON = 1 << 3,
    NES_DPAD_UP = 1 << 4,
    NES_DPAD_DOWN = 1 << 5,
    NES_DPAD_LEFT = 1 << 6,
    NES_DPAD_RIGHT = 1 << 7,
};

uint32_t sdl_poll_events() {
    SDL_Event sdl_event;
    uint32_t event = 0;

    while (SDL_PollEvent(&sdl_event)) {
        if (sdl_event.type == SDL_QUIT) {
            event |= WINDOW_QUIT;
        }
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_X]) {
            event |= NES_A_BUTTON;
        }
        if (state[SDL_SCANCODE_Z]) {
            event |= NES_B_BUTTON;
        }
        if (state[SDL_SCANCODE_RSHIFT]) {
            event |= NES_SELECT_BUTTON;
        }
        if (state[SDL_SCANCODE_RETURN]) {
            event |= NES_START_BUTTON;
        }
        if (state[SDL_SCANCODE_UP]) {
            event |= NES_DPAD_UP;
        }
        if (state[SDL_SCANCODE_DOWN]) {
            event |= NES_DPAD_DOWN;
        }
        if (state[SDL_SCANCODE_LEFT]) {
            event |= NES_DPAD_LEFT;
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            event |= NES_DPAD_RIGHT;
        }
    }

    return event;
}

void sdl_instance_destroy(SDLInstance *sdl_instance) {
    if (sdl_instance->pixel_buffer)
        free(sdl_instance->pixel_buffer);
    if (sdl_instance->texture)
        SDL_DestroyTexture(sdl_instance->texture);
    if (sdl_instance->renderer)
        SDL_DestroyRenderer(sdl_instance->renderer);
    if (sdl_instance->window)
        SDL_DestroyWindow(sdl_instance->window);
    SDL_Quit();
}

void sdl_put_pixel_region(SDLInstance *sdl_instance, WindowRegion *window_region, int relative_x, int relative_y, uint32_t color) {
    int x = relative_x, y = relative_y;

    // Bounds checking for the window region
    if (x < 0 || (x + 1) * window_region->scale_factor > window_region->width ||
        y < 0 || (y + 1) * window_region->scale_factor > window_region->height)
        return;

    uint32_t screen_offset_x = window_region->left_coord + x * window_region->scale_factor;
    uint32_t screen_offset_y = window_region->top_coord + y * window_region->scale_factor;

    for (int i = 0; i < window_region->scale_factor; i++) {
        for (int j = 0; j < window_region->scale_factor; j++) {
            sdl_put_pixel(sdl_instance, screen_offset_x + i, screen_offset_y + j, color);
        }
    }
}
