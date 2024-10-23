
#include "ppu.h"
#include "emulator.h"

// --------------- STATIC FORWARD DECLARATIONS ---------------- //
static void increment_scroll_x(PPU *ppu);
static void increment_scroll_y(PPU *ppu);
static void reload_scroll_x(PPU *ppu);
static void reload_scroll_y(PPU *ppu);
static void load_shifters(PPU *ppu);
static void update_shifters(PPU *ppu);
uint16_t calculate_vram_index(Mapper *mapper, uint16_t address);
static uint32_t get_color_from_palette(PPU *ppu, uint8_t palette, uint8_t pixel);
static void prepare_background_tile(PPU *ppu);
static void draw_background_pixel(PPU *ppu);

// --------------- PUBLIC FUNCTIONS --------------------------- //
void ppu_init(Emulator *emulator) {
    PPU *ppu = &emulator->ppu;
    ppu->emulator = emulator;
    ppu_reset(ppu);
}

void ppu_reset(PPU *ppu) {
    ppu->crtl.reg = ppu->mask.reg = ppu->status.reg = 0x00;
    ppu->oam_addr = ppu->oam_data = 0x00;
    ppu->vram_addr.reg = ppu->temp_addr.reg = 0x0000;
    ppu->write_latch = ppu->data_read_buffer = ppu->fine_x = 0x00;
    ppu->cur_scanline = ppu->cur_dot = 0;
    ppu->frame_complete = 0;
    ppu->next_tile_id = ppu->next_tile_attr = 0x00;
    ppu->next_tile_lsb = ppu->next_tile_msb = 0x00;
    ppu->shifter_pattern_lo = ppu->shifter_attr_hi = 0x0000;
    ppu->shifter_attr_lo = ppu->shifter_attr_hi = 0x0000;
    ppu->cycle_counter = 0;
    memset(ppu->vram, 0, sizeof(ppu->vram));
    memset(ppu->palette, 0, sizeof(ppu->palette));
}

void ppu_run_cycle(PPU *ppu) {
    CPU *cpu = &ppu->emulator->cpu;

    // Handle visible scanlines (0 - 239)
    if (ppu->cur_scanline < 240) {
        if (ppu->cur_dot == 0) {
        } // IDLE

        else if (ppu->cur_dot < 258) { // VISIBLE DOTS
            prepare_background_tile(ppu);
            draw_background_pixel(ppu);

            if (ppu->cur_dot == 256) {
                increment_scroll_y(ppu);
            }

            if (ppu->cur_dot == 257) {
                load_shifters(ppu);
                reload_scroll_x(ppu);
            }
        }

        else if (ppu->cur_dot < 321) {
        } // IDLE

        else if (ppu->cur_dot < 337) { // Prepare next row
            prepare_background_tile(ppu);
        }

        else if (ppu->cur_dot < 341) {
        } // IDLE
    }

    // Handle post-render scanline (240)
    else if (ppu->cur_scanline == 240) {
    } // IDLE SCANLINE

    // Handle vblank scanlines (241 - 260)
    else if (ppu->cur_scanline < 261) { // vblank

        if (ppu->cur_scanline == 241 && ppu->cur_dot == 1) {
            // Set VBlank flag and trigger NMI if enabled
            ppu->status.vblank = TRUE;
            if (ppu->crtl.enable_nmi) {
                cpu_set_interrupt(cpu, NMI);
            }
        }

    }

    // Handle pre-render scanline (261)
    else {
        if (ppu->cur_dot == 0) {
        } // IDLE

        else if (ppu->cur_dot < 258) {
            if (ppu->cur_dot == 1) {
                // Clear VBlank and sprite zero hit flags
                ppu->status.vblank = FALSE;
                ppu->status.sprite_zero_hit = FALSE;
            }

            prepare_background_tile(ppu);

            if (ppu->cur_dot == 256) {
                increment_scroll_y(ppu);
            }

            if (ppu->cur_dot == 257) {
                load_shifters(ppu);
                reload_scroll_x(ppu);
            }
        }

        else if (ppu->cur_dot < 280) {
        } // IDLE

        else if (ppu->cur_dot < 305) {
            reload_scroll_y(ppu);
        }

        else if (ppu->cur_dot < 321) {
        } // IDLE

        else if (ppu->cur_dot < 337) { // Prepare first row of next frame
            prepare_background_tile(ppu);
        }

        // Skip a cycle on odd frames (NTSC only)
        if (ppu->cur_dot == 339 && ppu->emulator->cur_frame % 2 == 1 && ppu->mask.render_background) {
            ppu->cur_dot++;
        }
    }

    ppu->cycle_counter++;

    // Advance dot and scanline counters
    ppu->cur_dot++;
    if (ppu->cur_dot >= 341) {
        ppu->cur_dot = 0;
        ppu->cur_scanline++;
        if (ppu->cur_scanline >= 262) {
            ppu->cur_scanline = 0;
            ppu->frame_complete = 1;
            // printf("frame %u, PPU cycle count: %lu\n", ppu->emulator->cur_frame, ppu->cycle_counter);
            ppu->cycle_counter = 0;
        }
    }
}

void ppu_set_ctrl(PPU *ppu, uint8_t value) {
    ppu->crtl.reg = value;
    ppu->temp_addr.nametable_x = ppu->crtl.nametable_x;
    ppu->temp_addr.nametable_y = ppu->crtl.nametable_y;
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
        mapper->write_chr(mapper, address, value);
    }

    // Writing to VRAM
    else if (address < 0x3F00) { // Writing to nametable
        uint16_t vram_index = calculate_vram_index(mapper, address);
        ppu->vram[vram_index] = value;
    }

    // Writing to namet
    else if (address < 0x4000) {
        address = address & 0x1F;
        ppu->palette[address] = value;

        if (address % 4 == 0) {
            ppu->palette[address ^ 0x10] = value;
        }
    }

    ppu->vram_addr.reg += ppu->crtl.increment ? 32 : 1;
}

uint8_t ppu_read_vram_data(PPU *ppu) {
    Mapper *mapper = &ppu->emulator->mapper;

    // We mirror the entire PPU memory space
    // If 0x4000 <= ppu->v then we wrap around and start from 0x0000
    uint16_t address = ppu->vram_addr.reg & 0x3FFF;

    uint8_t prev_buffer = ppu->data_read_buffer;

    // Reading from CHR ROM (Pattern Tables)
    if (address < 0x2000) {
        ppu->data_read_buffer = mapper->read_chr(mapper, address);
    }

    // Reading from VRAM (Nametables)
    else if (address < 0x4000) {
        uint16_t vram_index = calculate_vram_index(mapper, address);
        ppu->data_read_buffer = ppu->vram[vram_index];
    }

    // Reading from Palette
    if (0x3F00 <= address && address < 0x4000) {
        address = address & 0x1F;
        return ppu->palette[address];
    }

    ppu->vram_addr.reg += ppu->crtl.increment ? 32 : 1;
    return prev_buffer;
}

uint8_t ppu_const_read_vram_data(const PPU *ppu, uint16_t address) {
    Mapper *mapper = &ppu->emulator->mapper;

    // We mirror the entire PPU memory space
    // If 0x4000 <= ppu->v then we wrap around and start from 0x0000
    address &= 0x3FFF;

    // Reading from CHR ROM (Pattern Tables)
    if (address < 0x2000) {
        return mapper->read_chr(mapper, address);
    }

    // Reading from VRAM (Nametables)
    if (address < 0x3F00) {
        uint16_t vram_index = calculate_vram_index(mapper, address);
        return ppu->vram[vram_index];
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
    if (!ppu->mask.render_background && !ppu->mask.render_sprites)
        return;

    if (ppu->vram_addr.coarse_x == 31) {
        ppu->vram_addr.coarse_x = 0;
        ppu->vram_addr.nametable_x = ~ppu->vram_addr.nametable_x;
    } else {
        ppu->vram_addr.coarse_x++;
    }
}

static void increment_scroll_y(PPU *ppu) {
    if (!ppu->mask.render_background && !ppu->mask.render_sprites)
        return;

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
    if (!ppu->mask.render_background && !ppu->mask.render_sprites)
        return;
    ppu->vram_addr.nametable_x = ppu->temp_addr.nametable_x;
    ppu->vram_addr.coarse_x = ppu->temp_addr.coarse_x;
}

static void reload_scroll_y(PPU *ppu) {
    if (!ppu->mask.render_background && !ppu->mask.render_sprites)
        return;
    ppu->vram_addr.fine_y = ppu->temp_addr.fine_y;
    ppu->vram_addr.nametable_y = ppu->temp_addr.nametable_y;
    ppu->vram_addr.coarse_y = ppu->temp_addr.coarse_y;
}

static void load_shifters(PPU *ppu) {
    ppu->shifter_pattern_lo = (ppu->shifter_pattern_lo & 0xFF00) | ppu->next_tile_lsb;
    ppu->shifter_pattern_hi = (ppu->shifter_pattern_hi & 0xFF00) | ppu->next_tile_msb;
    ppu->shifter_attr_lo = (ppu->shifter_attr_lo & 0xFF00) | ((ppu->next_tile_attr & 0b01) ? 0xFF : 0x00);
    ppu->shifter_attr_hi = (ppu->shifter_attr_hi & 0xFF00) | ((ppu->next_tile_attr & 0b10) ? 0xFF : 0x00);
}

static void update_shifters(PPU *ppu) {
    if (ppu->mask.render_background) {
        ppu->shifter_pattern_lo <<= 1;
        ppu->shifter_pattern_hi <<= 1;
        ppu->shifter_attr_lo <<= 1;
        ppu->shifter_attr_hi <<= 1;
    }
}

uint16_t calculate_vram_index(Mapper *mapper, uint16_t address) {
    if (address < 0x2000 || address > 0x3FFF)
        return 0;

    if (address > 0x3000)
        address -= 0x1000;

    int nametable_index = (address - 0x2000) / 0x0400;
    uint16_t base_address = mapper->nametable_map[nametable_index];
    uint16_t offset = (address - 0x2000) % 0x03FF;
    uint16_t vram_index = base_address + offset;

    return vram_index;
}

uint32_t get_color_from_palette(PPU *ppu, uint8_t palette, uint8_t pixel) {
    uint32_t index = ppu_const_read_vram_data(ppu, (0x3F00 + (palette << 2) + pixel)) & 0x3F;
    return nes_palette_rgb[index];
}

void prepare_background_tile(PPU *ppu) {
    update_shifters(ppu);

    switch (ppu->cur_dot % 8) {
    case 1: {
        // fetch next nametable tile id
        load_shifters(ppu);
        uint16_t addr = 0x2000 | (ppu->vram_addr.reg & 0x0FFF);
        ppu->next_tile_id = ppu_const_read_vram_data(ppu, addr);
        break;
    }
    case 3: {
        // fetch next tile attribute
        uint16_t addr = 0x23C0 | (ppu->vram_addr.nametable_y << 11 | ppu->vram_addr.nametable_x << 10 |
                                  ((ppu->vram_addr.coarse_y >> 2) << 3) | (ppu->vram_addr.coarse_x >> 2));
        ppu->next_tile_attr = ppu_const_read_vram_data(ppu, addr);
        if (ppu->vram_addr.coarse_y & 0x02)
            ppu->next_tile_attr >>= 4;
        if (ppu->vram_addr.coarse_x & 0x02)
            ppu->next_tile_attr >>= 2;
        ppu->next_tile_attr &= 0x03;
        break;
    }
    case 5: {
        // fetch next pattern table tile row (LSB)
        ppu->next_tile_lsb = ppu_const_read_vram_data(ppu, (ppu->crtl.pattern_background << 12) +
                                                               ((uint16_t)ppu->next_tile_id << 4) +
                                                               (ppu->vram_addr.fine_y) + 0);
        break;
    }
    case 7: {
        // fetch next pattern table tile row (MSB)
        ppu->next_tile_msb = ppu_const_read_vram_data(ppu, (ppu->crtl.pattern_background << 12) +
                                                               ((uint16_t)ppu->next_tile_id << 4) +
                                                               (ppu->vram_addr.fine_y) + 8);
        break;
    }
    case 0: // increment vram_addr to next nametable tile
        increment_scroll_x(ppu);
        break;
    }
}

void draw_background_pixel(PPU *ppu) {
    uint8_t bg_pixel = 0x00;
    uint8_t bg_palette = 0x00;

    if (ppu->mask.render_background) {
        uint16_t bit_mux = 0x8000 >> ppu->fine_x;

        uint8_t p0_pixel = (ppu->shifter_pattern_lo & bit_mux) > 0;
        uint8_t p1_pixel = (ppu->shifter_pattern_hi & bit_mux) > 0;
        bg_pixel = (p1_pixel << 1) | p0_pixel;

        uint8_t pal0 = (ppu->shifter_attr_lo & bit_mux) > 0;
        uint8_t pal1 = (ppu->shifter_attr_hi & bit_mux) > 0;
        bg_palette = (pal1 << 1) | pal0;
    }

    uint32_t color = get_color_from_palette(ppu, bg_palette, bg_pixel);

#ifdef RISC_V
    // TODO draw to VGA screen
#else
    sdl_put_pixel_nes_screen(ppu->cur_dot, ppu->cur_scanline, color);
#endif
}