#ifndef PPU_H
#define PPU_H

#include "common.h"

#define NTSC_SCANLINES_PER_FRAME 262
#define PAL_SCANLINES_PER_FRAME 311

#define VISIBLE_SCANLINES 240
#define DOTS_PER_SCANLINE 341
#define VISIBLE_DOTS_PER_SCANLINE 256

// clang-format off
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
// clang-format on

/**
 *  This is a bitfield for easy access into the PPU CTRL register ($2000)
 *
 *  src: https://www.nesdev.org/wiki/PPU_registers#PPUCTRL
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

/**
 *  This is a bitfield for easy access into the PPU MASK register ($2001)
 *
 *  src: https://www.nesdev.org/wiki/PPU_registers#PPUMASK
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

/**
 *  This is a bitfield for easy access into the PPU STATUS register ($2002)
 *
 *  src: https://www.nesdev.org/wiki/PPU_registers#PPUSTATUS
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

/**
 *  This is a bitfield for easy access into the PPU internal registers v, and t.
 *
 *  It uses loopy arithmetic.
 */
typedef union {
    struct {
        uint16_t coarse_x : 5;
        uint16_t coarse_y : 5;
        uint16_t nametable_x : 1;
        uint16_t nametable_y : 1;
        uint16_t fine_y : 3;
        uint16_t unused : 1;
    } __attribute__((packed));
    struct {
        uint8_t low;
        uint8_t high;
    } __attribute__((packed));
    uint16_t reg;
} VRAM_ADDR_REGISTER;

// Forward Declarations
typedef struct Emulator Emulator;

typedef struct PPU {
    // PPU Registers
    PPU_CTRL_REGISTER crtl;
    PPU_MASK_REGISTER mask;
    PPU_STATUS_REGISTER status;
    uint8_t oam_addr;
    uint8_t oam_data;
    VRAM_ADDR_REGISTER vram_addr;
    VRAM_ADDR_REGISTER temp_addr;
    uint8_t write_latch;
    uint8_t data_read_buffer;
    uint8_t fine_x;

    // Rendering State
    size_t cur_scanline;
    size_t cur_dot;
    uint8_t frame_complete;

    // Background rendering shifters
    uint8_t next_tile_id;
    uint8_t next_tile_attr;
    uint8_t next_tile_lsb;
    uint8_t next_tile_msb;
    uint16_t shifter_pattern_lo;
    uint16_t shifter_pattern_hi;
    uint16_t shifter_attr_lo;
    uint16_t shifter_attr_hi;

    // debug
    size_t cycle_counter;

    // PPU memory
    Emulator *emulator;
    uint8_t vram[0x2000];
    uint8_t palette[0x20];
} PPU;

/**
 *  Initializes the PPU
 *
 *  Sets most values to 0.
 */
void ppu_init(Emulator *emulator);
void ppu_reset(PPU *ppu);

/**
 *  Runs one cycle of the PPU.
 *
 *  Each cycle corresponds to one pixel one the screen.
 *  This function is called 89342 times per frame.
 *  It is called three times as often as `cpu_run_cycle`
 */
void ppu_run_cycle(PPU *ppu);

/**
 *  Sets the PPUCTRL register.
 *
 */
void ppu_set_ctrl(PPU *ppu, uint8_t value);

/**
 *  Reads the PPUSTATUS register.
 *
 *  It also sets the vBlank flag and write_latch register to 0.
 */
uint8_t ppu_read_status(PPU *ppu);

/**
 *  Sets one byte of the PPUSCROLL register.
 *
 *  This function toggles between setting the high and low byte.
 */
void ppu_set_scroll(PPU *ppu, uint8_t value);

/**
 *  Sets one byte of the PPUADDR register.
 *
 *  This function toggles between setting the high and low byte.
 */
void ppu_set_vram_addr(PPU *ppu, uint8_t half_address);

/**
 *  Writes to VRAM. The address is specified by vram_addr
 *
 *  The behavior of this function depends greatly on what address is being written to,
 *  and what mapper is being used.
 *
 *  It also increments vram_addr by either 32 or 1, depending on ppu->ctrl.increment
 */
void ppu_write_vram_data(PPU *ppu, uint8_t value);

/**
 *  Reads from VRAM. The address is specified by vram_addr
 *
 *  The behavior of this function depends greatly on what address is read from,
 *  and what mapper is being used.
 *
 *  If reading from Palette memory, the value is returned directly.
 *  If reading from Pattern Tables or Nametables, the previously read value is returned,
 *     and the currently read value gets buffered in ppu->data_read_buffer
 *
 *  It also increments vram_addr by either 32 or 1, depending on ppu->ctrl.increment
 */
uint8_t ppu_read_vram_data(PPU *ppu);

/**
 *  Reads from VRAM. Doesn't modify internal state.
 *
 *  The behavior of this function depends greatly on what address is read from,
 *  and what mapper is being used.
 *
 *  The value being read is returned directly, and is not buffered.
 */
uint8_t ppu_const_read_vram_data(const PPU *ppu, uint16_t address);

#endif
