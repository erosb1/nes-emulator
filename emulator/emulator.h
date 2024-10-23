#ifndef EMULATOR_H
#define EMULATOR_H

#include "common.h"
#include "cpu.h"
#include "mapper.h"
#include "mem.h"
#include "ppu.h"

/**
 *  This struct is the entire NES emulator
 *  It is the owner of the CPU, PPU, MEM and Mapper devices.
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
    uint8_t controller_input; // Shift register

    // State
    int is_running;
    uint32_t event;
    uint32_t cur_frame;
    uint32_t time_point_start;

    // Calculate framerate based on the last 60 frames
    uint32_t frame_times[60];
} Emulator;

/**
 *  Initializes the emulator and all of its components.
 *
 */
void emulator_init(Emulator *emulator, uint8_t *rom);

/**
 *  Runs the emulator.
 *
 *  It contains the main frame loop that the program will spend
 *  99,9% of its time inside.
 *
 */
void emulator_run(Emulator *emulator);

/**
 *  Tests the CPU using the `tests/nestest.nes` rom.
 *
 *  It runs the CPU for a maximum of 26554 cycles and outputs the
 *  execution trace to the console.
 *
 *  It also runs the PPU to check that the cycle timing is correct.
 *
 */
void emulator_nestest(Emulator *emulator);

#endif