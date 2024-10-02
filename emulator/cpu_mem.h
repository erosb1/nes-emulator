#ifndef CPU_MEM_H
#define CPU_MEM_H

#include "common.h"

// memory regions based on NES memory map
// src: https://www.nesdev.org/wiki/CPU_memory_map
#define RAM_END 0x0800
#define RAM_MIRROR_END 0x2000
#define PPU_REGISTER_END 0x2008
#define PPU_MIRROR_END 0x4000
#define APU_IO_REGISTER_END 0x4020
#define PRG_RAM_END 0x8000
#define PRG_ROM_END 0x10000

// calculate sizes of memory regions
#define RAM_SIZE (RAM_END)
#define RAM_MIRROR_SIZE (RAM_MIRROR_END - RAM_END) // excluded
#define PPU_REGISTER_SIZE (PPU_REGISTER_END - RAM_MIRROR_END)
#define PPU_MIRROR_SIZE (PPU_MIRROR_END - PPU_REGISTER_END) // excluded
#define APU_IO_REGISTER_SIZE (APU_IO_REGISTER_END - PPU_REGISTER_END)
#define PRG_RAM_SIZE (PRG_RAM_END - APU_IO_REGISTER_END)
#define PRG_ROM_SIZE (PRG_ROM_END - PRG_RAM_END)

// stack parameters
#define STACK_OFFSET 0x0100

typedef struct CPUMemory {
    uint8_t ram[RAM_SIZE];
    uint8_t ppu_reg[PPU_REGISTER_SIZE];
    uint8_t apu_io_reg[APU_IO_REGISTER_SIZE];
    uint8_t cartridge_ram[PRG_RAM_SIZE];
    uint8_t cartridge_rom[PRG_ROM_SIZE];
} CPUMemory;

void cpu_write_mem_8(CPUMemory *mem, uint16_t address, uint8_t value);
uint8_t cpu_read_mem_8(CPUMemory *mem, uint16_t address);

void cpu_write_mem_16(CPUMemory *mem, uint16_t address, uint16_t value);
uint16_t cpu_read_mem_16(CPUMemory *mem, uint16_t address);

typedef struct CPU CPU;

void push_stack_16(CPU *cpu, uint16_t value);
uint16_t pop_stack_16(CPU *cpu);

void push_stack_8(CPU *cpu, uint8_t value);
uint8_t pop_stack_8(CPU *cpu);

#endif // CPU_MEM_H
