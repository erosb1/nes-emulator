
#include "ppu.h"
#include "emulator.h"

// --------------- STATIC FORWARD DECLARATIONS ---------------- //
static void increment_scroll_x(PPU *ppu);
static void increment_scroll_y(PPU *ppu);
static void reload_scroll_x(PPU *ppu);
static void reload_scroll_y(PPU *ppu);
static void load_shifters(PPU *ppu);
static void update_shifters(PPU *ppu);
static uint16_t calculate_vram_index(Mapper *mapper, uint16_t address);
static uint32_t get_color_from_palette(PPU *ppu, uint8_t palette, uint8_t pixel);
#ifdef RISC_V
static uint8_t get_color_from_palette_8(PPU *ppu, uint8_t palette, uint8_t pixel);
#endif
static void prepare_background_tile(PPU *ppu);
static void draw_pixel(PPU *ppu);
static uint8_t reverse_bits(uint8_t n);

// --------------- PUBLIC FUNCTIONS --------------------------- //
void ppu_init(Emulator *emulator) {
    PPU *ppu = &emulator->ppu;
    ppu->emulator = emulator;
    ppu_reset(ppu);
}

void ppu_reset(PPU *ppu) {
    ppu->ctrl.reg = ppu->mask.reg = ppu->status.reg = 0x00;
    ppu->oam_addr = 0x00;
    ppu->vram_addr.reg = ppu->temp_addr.reg = 0x0000;
    ppu->write_latch = ppu->data_read_buffer = ppu->fine_x = 0x00;
    ppu->cur_scanline = ppu->cur_dot = 0;
    ppu->frame_complete = 0;
    ppu->next_tile_id = ppu->next_tile_attr = 0x00;
    ppu->next_tile_lsb = ppu->next_tile_msb = 0x00;
    ppu->shifter_pattern_lo = ppu->shifter_attr_hi = 0x0000;
    ppu->shifter_attr_lo = ppu->shifter_attr_hi = 0x0000;
    ppu->cycle_counter = 0;
    ppu->sprite_count = 0;
    ppu->sprite_zero_hit_possible = 0;
    ppu->sprite_zero_hit_rendering = 0;
    memset(ppu->vram, 0, sizeof(ppu->vram));
    memset(ppu->palette, 0, sizeof(ppu->palette));
    memset(ppu->oam, 0, sizeof(ppu->oam));
    memset(ppu->sprite_scanline, 0, sizeof(ppu->sprite_scanline));
}

void ppu_run_cycle(PPU *ppu) {
    CPU *cpu = &ppu->emulator->cpu;

    // Handle visible scanlines (0 - 239)
    if (ppu->cur_scanline < 240) {
        if (ppu->cur_dot == 0) {
        } // IDLE

        else if (ppu->cur_dot < 258) { // VISIBLE DOTS
            prepare_background_tile(ppu);
            draw_pixel(ppu);

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
            if (ppu->ctrl.enable_nmi) {
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
                ppu->status.vblank = FALSE;
                ppu->status.sprite_zero_hit = FALSE;
                ppu->status.sprite_overflow = FALSE;
                memset(ppu->sprite_shifter_pattern_lo, 0, sizeof(ppu->sprite_shifter_pattern_lo));
                memset(ppu->sprite_shifter_pattern_hi, 0, sizeof(ppu->sprite_shifter_pattern_hi));
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

    // Sprite rendering (scanline based)
    if (ppu->cur_dot == 257 && ppu->cur_scanline != 261) {
        memset(ppu->sprite_scanline, 0xFF, sizeof(ppu->sprite_scanline));
        ppu->sprite_count = 0;
        ppu->sprite_zero_hit_possible = FALSE;

        for (size_t oam_entity_index = 0; oam_entity_index < 64 && ppu->sprite_count < 9; oam_entity_index++) {
            uint8_t sprite_y = ppu->oam[oam_entity_index * 4];
            size_t y_diff = (ppu->cur_scanline - (size_t) sprite_y);
            if (y_diff < (ppu->ctrl.sprite_size ? 16 : 8)) {
                if (oam_entity_index == 0)
                    ppu->sprite_zero_hit_possible = TRUE;
                if (ppu->sprite_count < 8) {
                    memcpy(&ppu->sprite_scanline[ppu->sprite_count * 4], &ppu->oam[oam_entity_index * 4], 4);
                    ppu->sprite_count++;
                }
            }
        }
        ppu->status.sprite_overflow = (ppu->sprite_count > 8);
    }

    if (ppu->cur_dot == 340) {
        for (uint8_t i = 0; i < ppu->sprite_count; i++) {
            uint16_t sprite_pattern_addr_lo;
            uint8_t flipped_horizontal = ppu->sprite_scanline[i * 4 + 2] & 0x40;
            uint8_t flipped_vertical = ppu->sprite_scanline[i * 4 + 2] & 0x80;
            uint8_t sprite_y = ppu->sprite_scanline[i * 4];
            uint16_t y_diff = ppu->cur_scanline - (uint16_t)sprite_y;
            uint8_t sprite_id = ppu->sprite_scanline[i * 4 + 1];

            if (!ppu->ctrl.sprite_size) {
                // 8 pixel height
                uint16_t row = flipped_vertical ? (7 - y_diff) : y_diff;
                sprite_pattern_addr_lo = (ppu->ctrl.pattern_sprite << 12) | (sprite_id << 4) | row;
            } else {
                // 16 pixel height
                uint16_t cell = y_diff < 8 ? (sprite_id & 0xFE) : (sprite_id & 0xFE) + 1;
                uint16_t row = flipped_vertical ? (7 - (y_diff & 0x07)) : (y_diff & 0x07);
                sprite_pattern_addr_lo = ((sprite_id & 0x01) << 12) | (cell << 4) | row;
            }

            uint16_t sprite_pattern_addr_hi = sprite_pattern_addr_lo + 1;
            uint8_t sprite_pattern_bits_lo = ppu_const_read_vram_data(ppu, sprite_pattern_addr_lo);
            uint8_t sprite_pattern_bits_hi = ppu_const_read_vram_data(ppu, sprite_pattern_addr_hi);

            // if flipped horizontally we just reverse the bytes
            if (flipped_horizontal) {
                sprite_pattern_bits_lo = reverse_bits(sprite_pattern_bits_lo);
                sprite_pattern_bits_hi = reverse_bits(sprite_pattern_bits_hi);
            }

            ppu->sprite_shifter_pattern_lo[i] = sprite_pattern_bits_lo;
            ppu->sprite_shifter_pattern_hi[i] = sprite_pattern_bits_hi;
        }
    }
}

// src: https://www.nesdev.org/wiki/PPU_scrolling#$2000_(PPUCTRL)_write
void ppu_set_ctrl(PPU *ppu, uint8_t value) {
    ppu->ctrl.reg = value;
    ppu->temp_addr.nametable_x = ppu->ctrl.nametable_x;
    ppu->temp_addr.nametable_y = ppu->ctrl.nametable_y;
}

// src: https://www.nesdev.org/wiki/PPU_scrolling#$2002_(PPUSTATUS)_read
uint8_t ppu_read_status(PPU *ppu) {
    uint8_t status = ppu->status.reg;
    ppu->write_latch = 0x00;
    ppu->status.vblank = FALSE; // reset v_blank
    return status;
}

// src: https://www.nesdev.org/wiki/PPU_scrolling#$2005_(PPUSCROLL)_first_write_(w_is_0)
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

// src: https://www.nesdev.org/wiki/PPU_scrolling#$2006_(PPUADDR)_first_write_(w_is_0)
void ppu_set_vram_addr(PPU *ppu, uint8_t half_address) {
    if (ppu->write_latch == 0) {
        // First write: Set the high byte (bits 8-14)
        ppu->temp_addr.high = (half_address & 0x3F); // clears bit 15
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

    ppu->vram_addr.reg += ppu->ctrl.increment ? 32 : 1;
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

    ppu->vram_addr.reg += ppu->ctrl.increment ? 32 : 1;
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

void ppu_dma(PPU *ppu, uint8_t page) {
    MEM *mem = &ppu->emulator->mem;
    CPU *cpu = &ppu->emulator->cpu;

    uint8_t* ptr = mem_get_pointer(mem, (uint16_t)page << 8);
    if (ptr == NULL) {
        // TODO slow DMA
        printf("SLOW DMA TRANSFER NOT IMPLEMENTED");
        exit(EXIT_FAILURE);
    } else {
        memcpy(ppu->oam + ppu->oam_addr, ptr, 256 - ppu->oam_addr);
        if(ppu->oam_addr)
            memcpy(ppu->oam, ptr + (256 - ppu->oam_addr), ppu->oam_addr);
    }

    cpu->dma_cycles += 513 + (cpu->total_cycles & 1);
}

// --------------- STATIC FUNCTIONS --------------------------- //

// src: https://www.nesdev.org/wiki/PPU_scrolling#Coarse_X_increment
static void increment_scroll_x(PPU *ppu) {
    if (!ppu->mask.render_background && !ppu->mask.render_sprites)
        return;

    if (ppu->vram_addr.coarse_x == 31) {
        ppu->vram_addr.coarse_x = 0;
        ppu->vram_addr.nametable_x ^= ppu->vram_addr.nametable_x;
    } else {
        ppu->vram_addr.coarse_x++;
    }
}

// src: https://www.nesdev.org/wiki/PPU_scrolling#Y_increment
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

    if (ppu->mask.render_sprites && 1 <= ppu->cur_dot && ppu->cur_dot < 258) {
        for (int i = 0; i < ppu->sprite_count; i++) {
            if (ppu->sprite_scanline[i * 4 + 3] > 0)
            {
                ppu->sprite_scanline[i * 4 + 3]--;
            }
            else
            {
                uint8_t pattern_lo = ppu->sprite_shifter_pattern_lo[i];
                uint8_t pattern_hi = ppu->sprite_shifter_pattern_hi[i];
                ppu->sprite_shifter_pattern_lo[i] = pattern_lo << 1;
                ppu->sprite_shifter_pattern_hi[i] = pattern_hi << 1;
            }
        }
    }
}

static uint16_t calculate_vram_index(Mapper *mapper, uint16_t address) {
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

static uint32_t get_color_from_palette(PPU *ppu, uint8_t palette, uint8_t pixel) {
    uint32_t index = ppu_const_read_vram_data(ppu, (0x3F00 + (palette << 2) + pixel)) & 0x3F;
    return nes_palette_rgb[index];
}

#ifdef RISC_V
static uint8_t get_color_from_palette_8(PPU *ppu, uint8_t palette, uint8_t pixel) {
    uint32_t index = ppu_const_read_vram_data(ppu, (0x3F00 + (palette << 2) + pixel)) & 0x3F;
    return nes_palette_8bit[index];
}
#endif

static void prepare_background_tile(PPU *ppu) {
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
        ppu->next_tile_lsb = ppu_const_read_vram_data(ppu, (ppu->ctrl.pattern_background << 12) +
                                                               ((uint16_t)ppu->next_tile_id << 4) +
                                                               (ppu->vram_addr.fine_y) + 0);
        break;
    }
    case 7: {
        // fetch next pattern table tile row (MSB)
        ppu->next_tile_msb = ppu_const_read_vram_data(ppu, (ppu->ctrl.pattern_background << 12) +
                                                               ((uint16_t)ppu->next_tile_id << 4) +
                                                               (ppu->vram_addr.fine_y) + 8);
        break;
    }
    case 0: // increment vram_addr to next nametable tile
        increment_scroll_x(ppu);
        break;
    }
}

static void draw_pixel(PPU *ppu) {
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

    uint8_t sprite_pixel = 0x00;
    uint8_t sprite_palette = 0x00;
    uint8_t sprite_priority = 0x00;

    if (ppu->mask.render_sprites) {
        ppu->sprite_zero_hit_rendering = FALSE;

        for (int i = 0; i < ppu->sprite_count; i++) {
            uint8_t sprite_x = ppu->sprite_scanline[i * 4 + 3];
            uint8_t sprite_attr  = ppu->sprite_scanline[i * 4 + 2];

            if (sprite_x == 0) {
                uint8_t sprite_pixel_lo = (ppu->sprite_shifter_pattern_lo[i] & 0x80) > 0;
                uint8_t sprite_pixel_hi = (ppu->sprite_shifter_pattern_hi[i] & 0x80) > 0;
                sprite_pixel = (sprite_pixel_hi << 1) | sprite_pixel_lo;

                sprite_palette = (sprite_attr & 0x03) + 0x04;
                sprite_priority = (sprite_attr & 0x20) == 0;

                if (sprite_pixel != 0) {
                    if (i == 0) {
                        ppu->sprite_zero_hit_rendering = TRUE;
                    }
                    break;
                }
            }
        }
    }

    uint8_t pixel = 0x00;
    uint8_t palette = 0x00;

    if (sprite_pixel == 0 && bg_pixel == 0) {
        pixel = 0x00;
        palette = 0x00;
    }
    else if (sprite_pixel == 0 && bg_pixel > 0) {
        pixel = bg_pixel;
        palette = bg_palette;
    } else if (sprite_pixel > 0 && bg_pixel == 0) {
        pixel = sprite_pixel;
        palette = sprite_palette;
    } else {
        if (sprite_priority) {
            pixel = sprite_pixel;
            palette = sprite_palette;
        } else {
            pixel = bg_pixel;
            palette = bg_palette;
        }

        if (ppu->sprite_zero_hit_possible && ppu->sprite_zero_hit_rendering) {
            if (ppu->mask.render_background & ppu->mask.render_sprites) {
                if (ppu->mask.render_background_left & ppu->mask.render_sprites_left) {
                    if (9 <= ppu->cur_dot && ppu->cur_dot < 258) ppu->status.sprite_zero_hit = TRUE;
                } else {
                    if (1 <= ppu->cur_dot && ppu->cur_dot < 258) ppu->status.sprite_zero_hit = TRUE;
                }
            }
        }
    }

#ifdef RISC_V
    uint8_t color = get_color_from_palette_8(ppu, palette, pixel);
    vga_screen_put_pixel(ppu->cur_dot, ppu->cur_scanline, color);
#else
    uint32_t color = get_color_from_palette(ppu, palette, pixel);
    sdl_put_pixel_nes_screen(ppu->cur_dot, ppu->cur_scanline, color);
#endif
}

static uint8_t reverse_bits(uint8_t n) {
    n = (n & 0xF0) >> 4 | (n & 0x0F) << 4; // Swap halves
    n = (n & 0xCC) >> 2 | (n & 0x33) << 2; // Swap pairs
    n = (n & 0xAA) >> 1 | (n & 0x55) << 1; // Swap individual bits
    return n;
}