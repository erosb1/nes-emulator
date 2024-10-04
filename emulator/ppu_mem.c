#include "ppu_mem.h"
#include "common.h"
#include "util.h"

void ppu_write_mem_8(PPUMemory *mem, uint16_t address, uint8_t value) {
    if (address < CHR_ROM_END) {
        mem->cartridge_rom[address] = value;
        return;
    }

    if (address < VRAM_END) {
        mem->vram[address - CHR_ROM_END] = value;
        return;
    }

    if (address < VRAM_MIRROR_END) {
        mem->vram[address - VRAM_END] = value; // may be read-only (?)
        return;
    }

    if (address < PALETTE_CTRL_END) {
        mem->palette_ctrl[address - VRAM_MIRROR_END] = value;
        return;
    }

    printf("Error: Tried to write to illegal memory address: %ui", address);
    assert(FALSE);
}

uint8_t ppu_read_mem_8(PPUMemory *mem, uint16_t address) {
    if (address < CHR_ROM_END) {
        return mem->cartridge_rom[address];
    }

    if (address < VRAM_END) {
        return mem->vram[address - CHR_ROM_END];
    }

    if (address < VRAM_MIRROR_END) {
        return mem->vram[address - VRAM_END]; // rarely used, if ever
    }

    if (address < PALETTE_CTRL_END) {
        return mem->palette_ctrl[address - VRAM_MIRROR_END];
    }

    printf("Tried to read from illegal memory address: %ui", address);
    assert(FALSE);
    return 0;
}

void ppu_write_mem_16(PPUMemory *mem, uint16_t address, uint16_t value) {
    ppu_write_mem_8(mem, address + 1, value >> BYTE_SIZE);
    ppu_write_mem_8(mem, address, value);
}

uint16_t ppu_read_mem_16(PPUMemory *mem, uint16_t address) {
    return (ppu_read_mem_8(mem, address + 1) << BYTE_SIZE) |
           ppu_read_mem_8(mem, address);
}
