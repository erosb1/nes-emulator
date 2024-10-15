
#include "ppu.h"
#include "util.h"
#include "gfx.h"
#include "emulator.h"

void init_ppu(Emulator *emulator) {
    PPU *ppu = &emulator->ppu;
    ppu->ppu_mem = &emulator->ppu_mem;

    ppu->cur_scanline = 0;
    ppu->cur_dot = 0;
}

void ppu_run_cycle(PPU *ppu) {
    if (ppu->cur_scanline < VISIBLE_SCANLINES) {
        // 0 <= cur_scanline < 240
        // Render background and sprites for scanlines 0-239
    }
    else if(ppu->cur_scanline == VISIBLE_SCANLINES){
        // cur_scanline == 240
        // Post-render scanline, typically idle
    }
    else if(ppu->cur_scanline < NTSC_SCANLINES_PER_FRAME) {
        // V-Blank period (scanlines 241-260 in NTSC)
        if (ppu->cur_scanline == VISIBLE_SCANLINES + 1 && ppu->cur_dot == 1) {
            // Set v-blank flag and possibly trigger NMI
            ppu->status.bits.vertical_blank = TRUE;
        }
    }
    else {
        // Pre-render scanline (scanline 261 in NTSC)
        if (ppu->cur_dot == 1) {
            // Reset v-blank and sprite zero hit flags
            ppu->status.bits.vertical_blank = FALSE;
            ppu->status.bits.sprite_zero_hit = FALSE;
        }
        if (ppu->cur_dot == 339 && ppu->cur_frame % 2 == 1) {
            // Skip a cycle on odd frames (NTSC only)
            ppu->cur_dot++;
        }
    }
}