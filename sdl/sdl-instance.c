
#include "sdl-instance.h"
#include "emulator.h"

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

void sdl_nes_put_pixel(SDLInstance *sdl_instance, int x, int y, uint32_t color) {
    if (0 <= x && x < SDL_NES_SCREEN_WIDTH && 0 <= y && y < SDL_NES_SCREEN_HEIGHT) {
        int screen_offset_x = SDL_NES_SCREEN_OFFSET_X + x * SDL_NES_SCALE_FACTOR;
        int screen_offset_y = SDL_NES_SCREEN_OFFSET_Y + y * SDL_NES_SCALE_FACTOR;

        // Draw a block of size (SDL_NES_SCALE_FACTOR x SDL_NES_SCALE_FACTOR)
        for (int i = 0; i < SDL_NES_SCALE_FACTOR; i++) {
            for (int j = 0; j < SDL_NES_SCALE_FACTOR; j++) {
                sdl_put_pixel(sdl_instance, screen_offset_x + i, screen_offset_y + j, color);
            }
        }
    }
}

static void put_debug_screen_pixel(SDLInstance *sdl_instance, int x, int y, uint32_t color) {
    int screen_offset_x = SDL_DEBUG_SCREEN_OFFSET_X + x * SDL_DEBUG_SCREEN_SCALE_FACTOR;
    int screen_offset_y = SDL_DEBUG_SCREEN_OFFSET_Y + y * SDL_DEBUG_SCREEN_SCALE_FACTOR;

    // Draw a block of size (SDL_NES_SCALE_FACTOR x SDL_NES_SCALE_FACTOR)
    for (int i = 0; i < SDL_DEBUG_SCREEN_SCALE_FACTOR; i++) {
        for (int j = 0; j < SDL_DEBUG_SCREEN_SCALE_FACTOR; j++) {
            sdl_put_pixel(sdl_instance, screen_offset_x + i, screen_offset_y + j, color);
        }
    }
}

static uint32_t get_color(uint8_t color_index) {
    switch (color_index) {
    case 1: return 0x555555;
    case 2: return 0xAAAAAA;
    case 3: return 0xFFFFFF;
    default: return 0x000000;
    }
}

static void render_pattern_table(SDLInstance *sdl_instance, PPU *ppu, int screen_offset_x, int screen_offset_y, int pattern_table_index) {
    const int TILE_SIZE = 16;
    const int TILE_WIDTH = 8;

    const uint16_t start_address = pattern_table_index ? 0x1000 : 0x0000;
    const uint16_t end_address   = pattern_table_index ? 0x2000 : 0x1000;

    for (uint16_t address = start_address; address < end_address; address += TILE_SIZE) {
        int tile_x = (address / TILE_SIZE) % 16;
        int tile_y = (address / TILE_SIZE) / 16;

        for (uint8_t y = 0; y < TILE_WIDTH; y++) {
            uint8_t low_byte = 0; //ppu_read_mem_8(ppu_mem, address + y);
            uint8_t high_byte = 0; //ppu_read_mem_8(ppu_mem, address + y + 8);

            for (uint8_t x = 0; x < TILE_WIDTH; x++) {
                uint8_t low_bit = (low_byte >> (7 - x)) & 1;
                uint8_t high_bit = (high_byte >> (7 - x)) & 1;
                uint8_t color_index = (high_bit << 1) | low_bit;

                uint32_t color = get_color(color_index);

                int pixel_x = screen_offset_x + (tile_x * TILE_WIDTH + x);
                int pixel_y = screen_offset_y + (tile_y * TILE_WIDTH + y);

                if (color != 0) put_debug_screen_pixel(sdl_instance, pixel_x, pixel_y, color);
            }
        }
    }
}

void sdl_draw_debug_info(SDLInstance *sdl_instance, Emulator *emulator) {
    for (int i = 0; i < SDL_NES_SCREEN_WIDTH; i++)
        for (int j = 0; j < SDL_NES_SCREEN_HEIGHT; j++)
            sdl_nes_put_pixel(sdl_instance, i, j, 0xFFFFFF);

    render_pattern_table(sdl_instance, &emulator->ppu, 0, 0, 0);
    render_pattern_table(sdl_instance, &emulator->ppu, 0, 80, 1);
}