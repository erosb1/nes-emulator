#include "ppu_mem.h"
#include "common.h"
#include "util.h"
#include "emulator.h"

void init_ppu_mem(Emulator *emulator) {
    PPUMemory *ppu_mem = &emulator->ppu_mem;
    ppu_mem->mapper = &emulator->mapper;
}

void ppu_write_mem_8(PPUMemory *mem, uint16_t address, uint8_t value) {
    if (address < CHR_ROM_END) {
        // Read ONLY
        return;
    }

    // Handle VRAM (0x2000 - 0x3EFF, including mirrored addresses)
    if (address < VRAM_MIRROR_END) {
        // VRAM mirrors every 0x1000 bytes (0x2000-0x2FFF mirrors into 0x3000-0x3EFF)
        address = CHR_ROM_END + (address - CHR_ROM_END) % VRAM_SIZE;
        mem->vram[address - CHR_ROM_END] = value;
        return;
    }

    // Handle Palette RAM (0x3F00 - 0x3FFF, mirrored every 32 bytes)
    if (address < PALETTE_CTRL_END) {
        address = VRAM_MIRROR_END + (address % 32);
        mem->palette_ctrl[address - VRAM_MIRROR_END] = value;
        return;
    }

    printf("Error: Tried to write to illegal memory address: 0x%04X\n", address);
    exit(EXIT_FAILURE);
}

uint8_t ppu_read_mem_8(PPUMemory *mem, uint16_t address) {
    // Handle CHR ROM (0x0000 - 0x1FFF, read-only)
    if (address < CHR_ROM_END) {
        return mem->mapper->read_chr(mem->mapper, address);
    }

    // Handle VRAM (0x2000 - 0x3EFF, including mirrored addresses)
    if (address < VRAM_MIRROR_END) {
        address = CHR_ROM_END + (address - CHR_ROM_END) % VRAM_SIZE;
        return mem->vram[address - CHR_ROM_END];
    }

    // Handle Palette RAM (0x3F00 - 0x3FFF, mirrored every 32 bytes)
    if (address < PALETTE_CTRL_END) {
        address = VRAM_MIRROR_END + (address % 32);
        return mem->palette_ctrl[address - VRAM_MIRROR_END];
    }

    printf("Error: Tried to read from illegal memory address: 0x%04X\n", address);
    exit(EXIT_FAILURE);
}

void ppu_write_mem_16(PPUMemory *mem, uint16_t address, uint16_t value) {
    ppu_write_mem_8(mem, address + 1, value >> BYTE_SIZE);
    ppu_write_mem_8(mem, address, value);
}

uint16_t ppu_read_mem_16(PPUMemory *mem, uint16_t address) {
    return (ppu_read_mem_8(mem, address + 1) << BYTE_SIZE) |
           ppu_read_mem_8(mem, address);
}
