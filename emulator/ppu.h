#ifndef PPU_H
#define PPU_H

#include "common.h"

#define PPU_MEM_SIZE 0x4000 // 16KiB

struct PPU {
  uint8_t extra_cycle_active;
  uint8_t extra_cycle_vblank;
  uint8_t mem[PPU_MEM_SIZE];
};

#undef PPU_MEM_SIZE

#endif
