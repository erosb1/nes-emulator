
#include "ppu.h"
#include "emulator.h"

void init_ppu(Emulator *emulator) {
    PPU *ppu = &emulator->ppu;
    ppu->ppu_mem = &emulator->ppu_mem;
}