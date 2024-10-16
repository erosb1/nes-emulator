#ifndef PPU_H
#define PPU_H

#include "common.h"


#define NTSC_SCANLINES_PER_FRAME 262
#define PAL_SCANLINES_PER_FRAME 311

#define VISIBLE_SCANLINES 240
#define DOTS_PER_SCANLINE 341
#define VISIBLE_DOTS_PER_SCANLINE 256



static const uint32_t nes_palette_rgb[] = {
    0x545454, 0x001E74, 0x081090, 0x300088, 0x440064, 0x5C0030, 0x540400, 0x3C1800,
    0x202A00, 0x083A00, 0x004000, 0x003C00, 0x00323C, 0x000000, 0x000000, 0x000000,
    0x989698, 0x084CC4, 0x3032EC, 0x5C1EE4, 0x8814B0, 0xA01464, 0x982220, 0x783C00,
    0x545A00, 0x287200, 0x087C00, 0x007628, 0x006678, 0x000000, 0x000000, 0x000000,
    0xECEEEC, 0x4C9AEC, 0x787CEC, 0xB062EC, 0xE454EC, 0xEC58B4, 0xEC6A64, 0xD48820,
    0xA0AA00, 0x74C400, 0x4CD020, 0x38CC6C, 0x38B4CC, 0x3C3C3C, 0x000000, 0x000000,
    0xECEEEC, 0xA8CCEC, 0xBCBCEC, 0xD4B2EC, 0xECAED4, 0xECA4A0, 0xECAE80, 0xECC280,
    0xD4C478, 0xB4DE88, 0xA8E2A8, 0xA8E2CC, 0xA8D4E2, 0xA8A8A8, 0x000000, 0x000000,
};



/*
 * This is a bitfield for easy access into the PPU CTRL register ($2000)
 *
 * src: https://www.nesdev.org/wiki/PPU_registers#PPUCTRL
 */
typedef union {
    struct {
        uint8_t nametable_x : 1;
        uint8_t nametable_y : 1;
        uint8_t increment : 1;
        uint8_t pattern_sprite : 1;
        uint8_t pattern_background : 1;
        uint8_t sprite_size : 1;
        uint8_t slave_mode : 1;
        uint8_t enable_nmi : 1;
    } __attribute__((packed));
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
    } __attribute__((packed));
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
        uint8_t vblank : 1;
    } __attribute__((packed));
    uint8_t reg;
} PPU_STATUS_REGISTER;


// Forward Declarations
typedef struct Emulator Emulator;
typedef struct Mapper Mapper;

typedef struct PPU {
    // PPU Registers (for communication with CPU)
    // On hardware these are located on memory addresses $2000 to $2007
    PPU_CTRL_REGISTER control;
    PPU_MASK_REGISTER mask;
    PPU_STATUS_REGISTER status;
    uint8_t oam_addr;
    uint8_t oam_data;

    // Internal registers
    uint16_t v;
    uint16_t t;
    uint8_t x;
    uint8_t w;
    uint8_t data_read_buffer;

    // Internal state
    size_t cur_scanline;
    size_t cur_dot;
    size_t cur_frame;

    // PPU memory
    Mapper *mapper;
    uint8_t vram[0x1000];
} PPU;

void ppu_init(Emulator *emulator);
void ppu_run_cycle(PPU *ppu);

uint8_t ppu_read_status(PPU *ppu);
void ppu_set_vram_addr(PPU* ppu, uint8_t half_address);
void ppu_write_vram_data(PPU *ppu, uint8_t value);
uint8_t ppu_read_vram_data(PPU *ppu);

#endif
