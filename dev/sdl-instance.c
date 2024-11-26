
#include "sdl-instance.h"
#include "common.h"

// Global SDLInstance variable
SDLInstance SDL_INSTANCE;

// Setup window regions
#define NES_SCREEN_SCALE_FACTOR 3

#define DEBUG_SCREEN_SCALE_FACTOR 1
#define DEBUG_SCREEN_WIDTH 256
#define DEBUG_SCREEN_HEIGHT SDL_WINDOW_HEIGHT

WindowRegion NES_SCREEN = {
    .top_coord = 0,
    .left_coord = 0,
    .width = NES_SCREEN_WIDTH * NES_SCREEN_SCALE_FACTOR,
    .height = NES_SCREEN_HEIGHT * NES_SCREEN_SCALE_FACTOR,
    .scale_factor = NES_SCREEN_SCALE_FACTOR,
};

WindowRegion DEBUG_SCREEN = {
    .top_coord = 0,
    .left_coord = 768,
    .width = DEBUG_SCREEN_WIDTH * DEBUG_SCREEN_SCALE_FACTOR,
    .height = DEBUG_SCREEN_HEIGHT * DEBUG_SCREEN_SCALE_FACTOR,
    .scale_factor = DEBUG_SCREEN_SCALE_FACTOR,
};

int sdl_instance_init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_INSTANCE.window = SDL_CreateWindow(SDL_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                           SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (SDL_INSTANCE.window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_INSTANCE.renderer = SDL_CreateRenderer(SDL_INSTANCE.window, -1, SDL_RENDERER_ACCELERATED);
    if (!SDL_INSTANCE.renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(SDL_INSTANCE.window);
        return -1;
    }

    SDL_INSTANCE.texture = SDL_CreateTexture(SDL_INSTANCE.renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT);

    SDL_INSTANCE.title = SDL_WINDOW_TITLE;
    SDL_INSTANCE.pixel_buffer = (uint32_t *)malloc(SDL_WINDOW_WIDTH * SDL_WINDOW_HEIGHT * sizeof(uint32_t));
    SDL_INSTANCE.width = SDL_WINDOW_WIDTH;
    SDL_INSTANCE.height = SDL_WINDOW_HEIGHT;

    return 0;
}

void sdl_clear_screen() {
    for (int i = 0; i < SDL_INSTANCE.width * SDL_INSTANCE.height; i++) {
        SDL_INSTANCE.pixel_buffer[i] = 0x00000000;
    }
}

void sdl_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= 0 && x < SDL_INSTANCE.width && y >= 0 && y < SDL_INSTANCE.height) {
        SDL_INSTANCE.pixel_buffer[y * SDL_INSTANCE.width + x] = color;
    }
}

void sdl_draw_frame() {
    SDL_UpdateTexture(SDL_INSTANCE.texture, NULL, SDL_INSTANCE.pixel_buffer, SDL_INSTANCE.width * sizeof(uint32_t));
    SDL_RenderClear(SDL_INSTANCE.renderer);
    SDL_RenderCopy(SDL_INSTANCE.renderer, SDL_INSTANCE.texture, NULL, NULL);
    SDL_RenderPresent(SDL_INSTANCE.renderer);
}

enum {
    NES_DPAD_RIGHT = 1 << 0,
    NES_DPAD_LEFT = 1 << 1,
    NES_DPAD_DOWN = 1 << 2,
    NES_DPAD_UP = 1 << 3,
    NES_START_BUTTON = 1 << 4,
    NES_SELECT_BUTTON = 1 << 5,
    NES_B_BUTTON = 1 << 6,
    NES_A_BUTTON = 1 << 7,
};

uint8_t sdl_poll_events() {
    SDL_Event sdl_event;
    uint8_t event = 0;

    while (SDL_PollEvent(&sdl_event)) {
    }

    const uint8_t *state = SDL_GetKeyboardState(NULL);
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

    return event;
}
void sdl_set_window_title(const char *title) { SDL_SetWindowTitle(SDL_INSTANCE.window, title); }

int sdl_window_quit() {
    SDL_Event sdl_event;

    while (SDL_PollEvent(&sdl_event)) {
        if (sdl_event.type == SDL_QUIT) {
            return 1;
        }
    }
    return 0;
}

void sdl_instance_destroy() {
    if (SDL_INSTANCE.pixel_buffer)
        free(SDL_INSTANCE.pixel_buffer);
    if (SDL_INSTANCE.texture)
        SDL_DestroyTexture(SDL_INSTANCE.texture);
    if (SDL_INSTANCE.renderer)
        SDL_DestroyRenderer(SDL_INSTANCE.renderer);
    if (SDL_INSTANCE.window)
        SDL_DestroyWindow(SDL_INSTANCE.window);
    SDL_Quit();
}

void sdl_put_pixel_region(WindowRegion *window_region, int relative_x, int relative_y, uint32_t color) {
    int x = relative_x, y = relative_y;

    // Bounds checking for the window region
    if (x < 0 || (x + 1) * window_region->scale_factor > window_region->width || y < 0 ||
        (y + 1) * window_region->scale_factor > window_region->height)
        return;

    uint32_t screen_offset_x = window_region->left_coord + x * window_region->scale_factor;
    uint32_t screen_offset_y = window_region->top_coord + y * window_region->scale_factor;

    for (int i = 0; i < window_region->scale_factor; i++) {
        for (int j = 0; j < window_region->scale_factor; j++) {
            sdl_put_pixel(screen_offset_x + i, screen_offset_y + j, color);
        }
    }
}

void sdl_put_pixel_nes_screen(int x, int y, uint32_t color) { sdl_put_pixel_region(&NES_SCREEN, x, y, color); }
