#ifndef CPU_MEMORY_H
#define CPU_MEMORY_H

#include "common.h"

// Memory regions based on NES memory map
// src: https://www.nesdev.org/wiki/CPU_memory_map
#define RAM_END              0x0800
#define RAM_MIRROR_END       0x2000
#define PPU_REGISTER_END     0x2008
#define PPU_MIRROR_END       0x4000
#define APU_IO_REGISTER_END  0x4020
#define CARTRIDGE_RAM_END    0x8000
#define CARTRIDGE_ROM_END    0xFFFF

// Calculate sizes of memory regions
#define RAM_SIZE             (RAM_END)
#define RAM_MIRROR_SIZE      (RAM_MIRROR_END - RAM_END) // Excluded
#define PPU_REGISTER_SIZE    (PPU_REGISTER_END - RAM_MIRROR_END)
#define PPU_MIRROR_SIZE      (PPU_MIRROR_END - PPU_REGISTER_END) // Excluded
#define APU_IO_REGISTER_SIZE (APU_IO_REGISTER_END - PPU_REGISTER_END)
#define CARTRIDGE_RAM_SIZE   (CARTRIDGE_RAM_END - APU_IO_REGISTER_END)
#define CARTRIDGE_ROM_SIZE   (CARTRIDGE_ROM_END - CARTRIDGE_RAM_END)

// Enums for easy access into PPU Registers (0x2000 - 0x2007)
typedef enum PPURegisters {
    PPU_CTRL = 0x2000,
    PPU_MASK,
    PPU_STATUS,
    OAM_ADDR,
    OAM_DATA,
    PPU_SCROLL,
    PPU_ADDR,
    PPU_DATA
} PPURegisters;

// Todo: Implement enums for access into APU and I/O registers

typedef struct CPUMemory {
    uint8_t ram[RAM_SIZE];
    uint8_t ppu_reg[PPU_REGISTER_SIZE];
    uint8_t apu_io_reg[APU_IO_REGISTER_SIZE];
    uint8_t cartridge_ram[CARTRIDGE_RAM_SIZE];
    uint8_t cartridge_rom[CARTRIDGE_ROM_SIZE];
} CPUMemory;


void init_memory(CPUMemory* mem);
void write_memory(CPUMemory* mem, uint16_t address, uint8_t value);
uint8_t read_memory(CPUMemory* mem, uint16_t address);

#endif //CPU_MEMORY_H
