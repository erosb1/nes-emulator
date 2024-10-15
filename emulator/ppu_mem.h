#ifndef PPU_MEM_H
#define PPU_MEM_H

#include "common.h"

// memory regions based on NES memory map
// https://www.nesdev.org/wiki/PPU_memory_map
#define CHR_ROM_END 0x2000
#define VRAM_END 0x3000
#define VRAM_MIRROR_END 0x3F00
#define PALETTE_CTRL_END 0x4000

// calculate sizes of memory regions
#define CHR_ROM_SIZE (CHR_ROM_END)
#define VRAM_SIZE (VRAM_END - CHR_ROM_END)
#define VRAM_MIRROR_SIZE (VRAM_MIRROR_END - VRAM_END) // excluded
#define PALETTE_CTRL_SIZE (PALETTE_CTRL_END - VRAM_END)

// Forward declarations
typedef struct Emulator Emulator;
typedef struct Mapper Mapper;

typedef struct PPUMemory {
    // CHR_ROM is accessed through the mapper
    uint8_t vram[VRAM_SIZE];
    uint8_t palette_ctrl[PALETTE_CTRL_SIZE];
    Mapper* mapper;
} PPUMemory;

void init_ppu_mem(Emulator *emulator);

void ppu_write_mem_8(PPUMemory *mem, uint16_t address, uint8_t value);
uint8_t ppu_read_mem_8(PPUMemory *mem, uint16_t address);

void ppu_write_mem_16(PPUMemory *mem, uint16_t address, uint16_t value);
uint16_t ppu_read_mem_16(PPUMemory *mem, uint16_t address);

#endif // PPU_MEM_H
