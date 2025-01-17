#ifndef CPU_MEM_H
#define CPU_MEM_H

#include "common.h"

// clang-format off
// memory regions based on NES memory map
// src: https://www.nesdev.org/wiki/CPU_memory_map
#define RAM_END                     0x0800
#define RAM_MIRROR_END              0x2000
#define PPU_REGISTER_END            0x2008
#define PPU_MIRROR_END              0x4000
#define APU_IO_REGISTER_END         0x4020
#define PRG_RAM_END                 0x8000
#define PRG_ROM_END                 0x10000

// calculate sizes of memory regions
#define RAM_SIZE                    (RAM_END)
#define RAM_MIRROR_SIZE             (RAM_MIRROR_END - RAM_END)
#define PPU_REGISTER_SIZE           (PPU_REGISTER_END - RAM_MIRROR_END)
#define PPU_MIRROR_SIZE             (PPU_MIRROR_END - PPU_REGISTER_END)
#define APU_IO_REGISTER_SIZE        (APU_IO_REGISTER_END - PPU_MIRROR_END)
#define PRG_RAM_SIZE                (PRG_RAM_END - APU_IO_REGISTER_END)
#define PRG_ROM_SIZE                (PRG_ROM_END - PRG_RAM_END)
// clang-format on

#define STACK_OFFSET 0x0100

// Forward declarations
typedef struct Emulator Emulator;
typedef struct CPU CPU;

typedef struct MEM {
    uint8_t ram[RAM_SIZE];
    // PPU registers are stored in the PPU struct
    uint8_t apu_io_reg[APU_IO_REGISTER_SIZE];
    uint8_t cartridge_ram[PRG_RAM_SIZE];
    // PRG_ROM is accessed through the mapper

    uint8_t controller_shift_register;

    Emulator *emulator;
} MEM;

/**
 *  Initializes memory.
 *
 *  Sets most values to 0
 */
void init_cpu_mem(Emulator *emulator);

/**
 *  Writes a byte to memory.
 *
 *  Also handles memory mirroring.
 */
void mem_write_8(MEM *mem, uint16_t address, uint8_t value);

/**
 *  Reads a byte from memory.
 *
 *  Als handles memory mirroring. This function
 *  might modify the state of other devices, especially
 *  if reading from PPU registers.
 */
uint8_t mem_read_8(MEM *mem, uint16_t address);

/**
 *  Writes two bytes to memory in little-endian.
 *
 *  Calls `mem_write_8` two times.
 */
void mem_write_16(MEM *mem, uint16_t address, uint16_t value);

/**
 *  Reads two bytes from memory in little-endian.
 *
 *  Calls `mem_read_8` two times.
 */
uint16_t mem_read_16(MEM *mem, uint16_t address);

/**
 *  Pushes a byte to the stack.
 *
 *  The stack is located in mem->ram, starts at 0x0100,
 *  and grows downwards.
 */
void mem_push_stack_8(CPU *cpu, uint8_t value);

/**
 *  Pops a byte from the stack.
 *
 *  The stack is located in mem->ram, starts at 0x0100,
 *  and grows downwards.
 */
uint8_t mem_pop_stack_8(CPU *cpu);

/**
 *  Pushes two bytes to the stack in little-endian.
 *
 */
void mem_push_stack_16(CPU *cpu, uint16_t value);

/**
 *  Pops two bytes from the stack in little-endian.
 *
 */
uint16_t pop_stack_16(CPU *cpu);

/**
 *  Reads a byte from memory, without modifying any external state.
 *
 *  This function is mainly used for debugging purposes.
 */
uint8_t mem_const_read_8(const MEM *mem, uint16_t address);

uint8_t* mem_get_pointer(MEM *mem, uint16_t address);

#endif // CPU_MEM_H
