#ifndef PPU_H
#define PPU_H

#include "common.h"
#include "ppu_mem.h"

#define PPU_MEM_SIZE 0x4000 // 16KiB

typedef struct PPU {
    uint8_t extra_cycle_active;
    uint8_t extra_cycle_vblank;
    PPUMemory *mem;
} PPU;

#undef PPU_MEM_SIZE

#endif
