
#include "ppu.h"
#include "emulator.h"

// --------------- STATIC FORWARD DECLARATIONS ---------------- //
static void increment_scroll_x(PPU *ppu);
static void increment_scroll_y(PPU *ppu);
static void reload_scroll_x(PPU *ppu);
static void reload_scroll_y(PPU *ppu);



// --------------- PUBLIC FUNCTIONS ---------- ---------------- //
void ppu_init(Emulator *emulator) {
    PPU *ppu = &emulator->ppu;
    ppu->emulator = emulator;
    ppu_reset(ppu);
}

void ppu_reset(PPU *ppu) {
    ppu->control.reg = ppu->mask.reg = ppu->status.reg = 0x00;
    ppu->oam_addr = ppu->oam_data = 0x00;
    ppu->vram_addr.reg = ppu->temp_addr.reg = 0x0000;
    ppu->x = ppu->write_latch = ppu->data_read_buffer = ppu->fine_x = 0x00;
    ppu->cur_scanline = ppu->cur_dot = 0;
    memset(ppu->vram, 0, sizeof(ppu->vram));
    memset(ppu->palette, 0, sizeof(ppu->palette));
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
        if (ppu->cur_dot == 339 && ppu->emulator->cur_frame % 2 == 1) {
            // Skip a cycle on odd frames (NTSC only)
            ppu->cur_dot++;
        }
    }
}

uint8_t ppu_read_status(PPU *ppu) {
    uint8_t status = ppu->status.reg;
    ppu->write_latch = 0x00;
    ppu->status.vblank = FALSE; // reset v_blank
    return status;
}

void ppu_set_scroll(PPU *ppu, uint8_t value) {
    if (ppu->write_latch == 0) {
        // First write: Set X scroll value
        ppu->fine_x = value & 0x07;
        ppu->temp_addr.coarse_x = value >> 3;
        ppu->write_latch = 1;
    } else {
        // Second write: Set Y scroll value
        ppu->temp_addr.fine_y = value & 0x07;
        ppu->temp_addr.coarse_y = value >> 3;
        ppu->write_latch = 0;
    }
}

void ppu_set_vram_addr(PPU *ppu, uint8_t half_address) {
    if (ppu->write_latch == 0) {
        // First write: Set the high byte (bits 8-14)
        ppu->temp_addr.high = (half_address & 0x3F);
        ppu->write_latch = 1;
    } else {
        // Second write: Set the low byte (bits 0-7)
        ppu->temp_addr.low = half_address;
        ppu->vram_addr = ppu->temp_addr;
        ppu->write_latch = 0;
    }
}

void ppu_write_vram_data(PPU *ppu, uint8_t value) {
    Mapper *mapper = &ppu->emulator->mapper;

    // We mirror the entire PPU memory space
    // If 0x4000 <= ppu->v then we wrap around and start from 0x0000
    uint16_t address = ppu->vram_addr.reg & 0x3FFF;

    // Writing to CHR ROM (Palette memory) is only allowed with certain mappers
    if (address < 0x2000) {
        // ppu->mapper.write_chr(ppu->mapper, address, value);
    }

    // Writing to VRAM
    else if (address < 0x3F00) { // Writing to nametable
        uint16_t mirrored_address = mapper_mirror_nametable_address(mapper, address);
        ppu->vram[mirrored_address] = value;
    }

    // Writing to Palette
    else if (address < 0x4000) {
        address = address & 0x1F;
        ppu->palette[address] = value;

        if (address % 4 == 0) {
            ppu->palette[address ^ 0x10] = value;
        }
    }

    ppu->vram_addr.reg += ppu->control.increment ? 32 : 1;
}

uint8_t ppu_read_vram_data(PPU *ppu) {
    Mapper *mapper = &ppu->emulator->mapper;

    // We mirror the entire PPU memory space
    // If 0x4000 <= ppu->v then we wrap around and start from 0x0000
    uint16_t address = ppu->vram_addr.reg & 0x3FFF;

    // We return the previous value we read (not if reading from palette)
    uint8_t return_value = ppu->data_read_buffer;

    // Reading from CHR ROM (Pattern Tables)
    if (address < 0x2000) {
        ppu->data_read_buffer = mapper->read_chr(mapper, address);
    }

    // Reading from VRAM (Nametables)
    else if (address < 0x3F00) {
        address = mapper_mirror_nametable_address(mapper, address);
        ppu->data_read_buffer = ppu->vram[address];
    }

    // Reading from Palette
    else if (address < 0x4000) {
        address = address & 0x1F;
        return_value = ppu->palette[address];
    }

    ppu->vram_addr.reg += ppu->control.increment ? 32 : 1;
    return return_value;
}

uint8_t ppu_const_read_vram_data(const PPU *ppu, uint16_t address) {
    Mapper *mapper = &ppu->emulator->mapper;

    // We mirror the entire PPU memory space
    // If 0x4000 <= ppu->v then we wrap around and start from 0x0000
    address &= 0x3FFF;

    // We return the previous value we read (not if reading from palette)
    uint8_t return_value = ppu->data_read_buffer;

    // Reading from CHR ROM (Pattern Tables)
    if (address < 0x2000) {
        return mapper->read_chr(mapper, address);
    }

    // Reading from VRAM (Nametables)
    if (address < 0x3F00) {
        address = mapper_mirror_nametable_address(mapper, address);
        return ppu->vram[address];
    }

    // Reading from Palette
    if (address < 0x4000) {
        address = address & 0x1F;
        return ppu->palette[address];
    }

    return 0;
}

// --------------- STATIC FUNCTIONS --------------------------- //
static void increment_scroll_x(PPU *ppu) {
    if (!ppu->mask.render_background && !ppu->mask.render_sprites) return;

    if (ppu->vram_addr.coarse_x == 31) {
        ppu->vram_addr.coarse_x = 0;
        ppu->vram_addr.nametable_x = ~ppu->vram_addr.nametable_x;
    } else {
        ppu->vram_addr.coarse_x++;
    }
}


static void increment_scroll_y(PPU *ppu) {
    if (!ppu->mask.render_background && !ppu->mask.render_sprites) return;

    if (ppu->vram_addr.fine_y < 7) {
        ppu->vram_addr.fine_y++;
        return;
    }

    ppu->vram_addr.fine_y = 0;

    if (ppu->vram_addr.coarse_y == 29) {
        ppu->vram_addr.coarse_y = 0;
        ppu->vram_addr.nametable_y = ~ppu->vram_addr.nametable_y;
    }

    else if (ppu->vram_addr.coarse_y == 31) {
        ppu->vram_addr.coarse_y = 0;
    }

    else {
        ppu->vram_addr.coarse_y++;
    }
}

static void reload_scroll_x(PPU *ppu) {
    if (!ppu->mask.render_background && !ppu->mask.render_sprites) return;
    ppu->vram_addr.nametable_x = ppu->temp_addr.nametable_x;
    ppu->vram_addr.coarse_x = ppu->temp_addr.coarse_x;

}

static void reload_scroll_y(PPU *ppu) {
    if (!ppu->mask.render_background && !ppu->mask.render_sprites) return;
    ppu->vram_addr.fine_y = ppu->temp_addr.fine_y;
    ppu->vram_addr.nametable_y = ppu->temp_addr.nametable_y;
    ppu->vram_addr.coarse_y = ppu->temp_addr.coarse_y;
}
