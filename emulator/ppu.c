
#include "ppu.h"
#include "util.h"
#include "emulator.h"

void ppu_init(Emulator *emulator) {
    PPU *ppu = &emulator->ppu;
    ppu->mapper = &emulator->mapper;

    ppu->cur_scanline = 0;
    ppu->cur_dot = 0;
}

void ppu_run_cycle(PPU *ppu) {
    if (ppu->cur_scanline < VISIBLE_SCANLINES) {
        // 0 <= cur_scanline < 240
        // Render background and sprites for scanlines 0-239
    } else if (ppu->cur_scanline == VISIBLE_SCANLINES) {
        // cur_scanline == 240
        // Post-render scanline, typically idle
    } else if (ppu->cur_scanline < NTSC_SCANLINES_PER_FRAME) {
        // V-Blank period (scanlines 241-260 in NTSC)
        if (ppu->cur_scanline == VISIBLE_SCANLINES + 1 && ppu->cur_dot == 1) {
            // Set v-blank flag and possibly trigger NMI
            ppu->status.vblank = TRUE;
        }
    } else {
        // Pre-render scanline (scanline 261 in NTSC)
        if (ppu->cur_dot == 1) {
            // Reset v-blank and sprite zero hit flags
            ppu->status.vblank = FALSE;
            ppu->status.sprite_zero_hit = FALSE;
        }
        if (ppu->cur_dot == 339 && ppu->cur_frame % 2 == 1) {
            // Skip a cycle on odd frames (NTSC only)
            ppu->cur_dot++;
        }
    }
}

uint8_t ppu_read_status(PPU *ppu) {
    uint8_t status = ppu->status.reg;
    ppu->w = 0x00;
    ppu->status.vblank = FALSE; // reset v_blank
    return status;
}

void ppu_set_vram_addr(PPU *ppu, uint8_t half_address) {
    if (ppu->w == 0) {
        // First write: Set the high byte (bits 8-14)
        ppu->t = (ppu->t & 0x00FF) | ((half_address & 0x3F) << 8);
        ppu->w = 1;
    }
    else {
        // Second write: Set the low byte (bits 0-7)
        ppu->t = (ppu->t & 0x7F00) | half_address;
        ppu->v = ppu->t;
        ppu->w = 0;
    }
}

void ppu_write_vram_data(PPU *ppu, uint8_t value) {
    uint16_t address = ppu->v & 0x3FFF;

    // Writing to CHR ROM (Palette memory) is only allowed with certain mappers
    if (address < 0x2000) {
        // ppu->mapper.write_chr(ppu->mapper, address, value);
    }

    // Writing to VRAM
    else if (address < 0x3F00) { // Writing to nametable
        address = (address & 0xefff) - 0x2000;
        ppu->vram[address] = value;
        // Todo: implement nametable mirroring depending on mapper
    }

    // Writing to Palette
    else if (address < 0x4000) {
        address = address & 0x1F;
        // Todo: handle palette writing
    }

    ppu->v += ppu->control.increment ? 32 : 1;
}

uint8_t ppu_read_vram_data(PPU *ppu) {
    // We mirror the entire PPU memory space
    // If 0x4000 <= ppu->v then we wrap around and start from 0x0000
    uint16_t address = ppu->v & 0x3FFF;

    // We return the previous value we read (not if reading from palette=
    uint8_t prev_data = ppu->data_read_buffer;

    // Reading from CHR ROM (palette memory)
    if (address < 0x2000) {
        ppu->data_read_buffer = ppu->mapper->read_chr(ppu->mapper, address);
    }

    // Reading from VRAM
    else if (address < 0x3F00) { // Writing to nametable
        address = (address & 0xefff) - 0x2000;
        ppu->data_read_buffer = ppu->vram[address];
        // Todo: implement nametable mirroring depending on mapper
    }

    // Reading from Palette
    else if (address < 0x4000) {
        address = address & 0x1F;
        // Todo: handle palette writing (reading from here is not buffered)
    }

    ppu->v += ppu->control.increment ? 32 : 1;

    return prev_data;
}