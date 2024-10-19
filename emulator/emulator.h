#ifndef EMULATOR_H
#define EMULATOR_H

#include "common.h"
#include "cpu.h"
#include "mapper.h"
#include "mem.h"
#include "ppu.h"

/*
 * This struct is the entire NES emulator
 * It is the owner of the CPU, PPU, MEM and Mapper devices.
 *
 */
typedef struct Emulator {
    // Rom reference (non-owning)
    uint8_t *rom;

    // Devices
    CPU cpu;
    PPU ppu;
    MEM mem;
    Mapper mapper;

    // State
    int is_running;
    uint32_t event;
    uint32_t cur_frame;
    uint32_t time_point_start;

    // Calculate framerate based on the last 60 frames
    uint32_t frame_times[60];
} Emulator;

/*
 * This function initialises the emulator and all of its components.
 *
 */
void emulator_init(Emulator *emulator, uint8_t *rom);

/*
 * This function runs the emulator.
 * The program will spend 99.99% of its execution time inside of this function.
 *
 */
void emulator_run(Emulator *emulator);

/*
 * This function tests the CPU using the `tests/nestest.nes` rom.
 *
 * It runs the CPU for a maximum of 26554 cycles and outputs the
 * execution trace to the console.
 *
 * As of now, it doesn't do anything with the PPU (this might change later)
 *
 */
void emulator_nestest(Emulator *emulator);

#endif