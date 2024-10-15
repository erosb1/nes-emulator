#ifndef EMULATOR_H
#define EMULATOR_H

#include "common.h"
#include "cpu.h"
#include "ppu.h"
#include "cpu_mem.h"
#include "ppu_mem.h"
#include "mapper.h"



/*
 * This struct is the entire NES emulator
 * It is the owner of the CPU, PPU CPUMemory and PPUMemory devices.
 *
 */
typedef struct Emulator {
    // Rom reference (non-owning)
    uint8_t *rom;

    // Devices
    CPU cpu;
    PPU ppu;
    CPUMemory cpu_mem;
    PPUMemory ppu_mem;
    Mapper mapper;

    // Emulator status
    int is_running; // boolean value
} Emulator;

/*
 * This function initialises the emulator and all of its components.
 *
 * It also loads the PRG section of `rom_buffer` into `cpu_mem`,
 * and the CHR section of into `ppu_mem`.
 *
 */
void init_emulator(Emulator *emulator, uint8_t *rom);

/*
 * This function runs the emulator.
 * The program will spend 99.99% of it's execution time inside of this function.
 *
 */
void run_emulator(Emulator *emulator);

/*
 * This function tests the CPU using the `tests/nestest.nes` rom.
 *
 * It runs the CPU for a maximum of 26554 cycles and outputs the
 * execution trace to the console.
 *
 * As of now, it doesn't do anything with the PPU (this might change later)
 *
 */
void run_nestest(Emulator *emulator);

#endif