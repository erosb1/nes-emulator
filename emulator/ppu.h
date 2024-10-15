#ifndef PPU_H
#define PPU_H

#include "common.h"
#include "ppu_mem.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 224

#define NTSC_SCANLINES_PER_FRAME 262
#define PAL_SCANLINES_PER_FRAME 311

#define VISIBLE_SCANLINES 240
#define DOTS_PER_SCANLINE 341
#define VISIBLE_DOTS_PER_SCANLINE 256

/*
 * This is a bitfield for easy access into the PPU CTRL register ($2000)
 *
 * src: https://www.nesdev.org/wiki/PPU_registers#PPUCTRL
 */
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

/*
 * This is a bitfield for easy access into the PPU MASK register ($2001)
 *
 * src: https://www.nesdev.org/wiki/PPU_registers#PPUMASK
 */
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

/*
 * This is a bitfield for easy access into the PPU STATUS register ($2002)
 *
 * src: https://www.nesdev.org/wiki/PPU_registers#PPUSTATUS
 */
typedef union {
    struct {
        uint8_t unused : 5;
        uint8_t sprite_overflow : 1;
        uint8_t sprite_zero_hit : 1;
        uint8_t vertical_blank : 1;
    } __attribute__((packed)) bits;
    uint8_t reg;
} PPU_STATUS_REGISTER;

/*
 * This is a struct for easy access into the 16 bit registers, i.e the PPUSCROLL and PPUADDR (vram_addr) registers
 *
 * These registers are write-only for the CPU, and needs two separate writes.
 * The first write is to the low byte and the second one is to the high byte.
 */
typedef union {
    struct {
        uint8_t low;
        uint8_t high;
    } bytes;
    uint16_t full;
} PPU_16_BIT_REGISTER;

// Forward Declarations
typedef struct Emulator Emulator;

typedef struct PPU {
    uint8_t extra_cycle_active;
    uint8_t extra_cycle_vblank;

    // PPU Registers (for communication with CPU)
    // On hardware these are located on memory addresses $2000 to $2007
    PPU_CTRL_REGISTER control;
    PPU_MASK_REGISTER mask;
    PPU_STATUS_REGISTER status;
    uint8_t oam_addr;
    uint8_t oam_data;
    PPU_16_BIT_REGISTER scroll;
    PPU_16_BIT_REGISTER vram_addr;
    uint8_t vram_data;
    int write_toggle;

    // Internal state
    size_t cur_scanline;
    size_t cur_dot;
    size_t cur_frame;

    // References to other components
    PPUMemory *ppu_mem;
} PPU;

void init_ppu(Emulator *emulator);
void ppu_run_cycle(PPU *ppu);

#endif
