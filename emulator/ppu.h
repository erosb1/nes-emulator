#ifndef PPU_H
#define PPU_H

#include "common.h"
#include "ppu_mem.h"

#define PPU_MEM_SIZE 0x4000 // 16KiB

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 224

#define NTSC_SCANLINES_PER_FRAME 262
#define NTSC_VISIBLE_SCANLINES 240
// Todo: Add support for PAL rendering
#define PIXELS_PER_SCANLINE 341
#define VISIBLE_PIXELS_PER_SCANLINE 256

typedef union {
    struct {
        uint8_t nametable_x : 1;
        uint8_t nametable_y : 1;
        uint8_t increment_mode : 1;
        uint8_t pattern_sprite : 1;
        uint8_t pattern_background : 1;
        uint8_t sprite_size : 1;
        uint8_t slave_mode : 1;
        uint8_t enable_nmi : 1;
    } __attribute__((packed)) bits;
    uint8_t reg;
} PPU_CTRL_REGISTER;

typedef union {
    struct {
        uint8_t grayscale : 1;
        uint8_t render_background_left : 1;
        uint8_t render_sprites_left : 1;
        uint8_t render_background : 1;
        uint8_t render_sprites : 1;
        uint8_t enhance_red : 1;
        uint8_t enhance_green : 1;
        uint8_t enhance_blue : 1;
    } __attribute__((packed)) bits;
    uint8_t reg;
} PPU_MASK_REGISTER;

typedef union {
    struct {
        uint8_t unused : 5;
        uint8_t sprite_overflow : 1;
        uint8_t sprite_zero_hit : 1;
        uint8_t vertical_blank : 1;
    } __attribute__((packed)) bits;
    uint8_t reg;
} PPU_STATUS_REGISTER;

typedef union {
    struct {
        uint8_t low;
        uint8_t high;
    } bytes;
    uint16_t full;
} PPU_16_BIT_REGISTER;

typedef struct PPU {

    // PPU Registers (for communication with CPU)
    PPU_CTRL_REGISTER control;
    PPU_MASK_REGISTER mask;
    PPU_STATUS_REGISTER status;
    uint8_t oam_addr;
    uint8_t oam_data;
    PPU_16_BIT_REGISTER scroll;
    PPU_16_BIT_REGISTER vram_addr;
    uint8_t vram_data;

    PPUMemory *mem;
} PPU;

#undef PPU_MEM_SIZE

#endif
