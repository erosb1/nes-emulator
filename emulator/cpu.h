#ifndef CPU_H
#define CPU_H

#include "common.h"

// forward declarations
typedef struct Emulator Emulator;

typedef enum Interrupt {
    NONE,
    NMI,
    IRQ,
    RSI,
} Interrupt;

// clang-format off
// enum shorthand for toggling CPU flags
typedef enum CPUFlag {
    CARRY      = (1 << 0),
    ZERO       = (1 << 1),
    INTERRUPT  = (1 << 2),
    DECIMAL    = (1 << 3),
    BREAK      = (1 << 4),
    UNUSED     = (1 << 5),
    OVERFLW    = (1 << 6),
    NEGATIVE   = (1 << 7),
} CPUFlag;
// clang-format on

typedef struct CPU {
    uint16_t pc;      // program counter
    uint16_t address; // addressing mode address (set before executing each instruction)
    uint8_t ac;       // accumulator
    uint8_t x;        // x register
    uint8_t y;        // y register
    uint8_t sr;       // status register [NV-BDIZC]
    uint8_t sp;       // stack pointer (wraps)
    size_t total_cycles;
    size_t cycles;
    Interrupt pending_interrupt;

    // References to other devices
    Emulator *emulator;

    int is_logging;
} CPU;

/**
 *  Initializes the CPU.
 *
 *  Most values are set to 0.
 */
void cpu_init(Emulator *emulator);

/**
 *  Executes a single CPU cycle.
 *
 *  In this emulator, full instruction execution occurs in the first cycle,
 *  as it is not cycle-accurate. The remaining cycles for the instruction are idle.
 */
void cpu_run_cycle(CPU *cpu);

/**
 *  Signals to the CPU to interrupt execution.
 *
 *  This function sets the pending_interrupt variable. The interrupts are handled at
 *  the start of the next instruction.
 *
 *  There are three interrupts available:
 *
 *  1. NMI (non maskable interrupt) - sent by the PPU at the start of vBlank.
 *       Can not be ignored by the CPU.
 *  2. IRQ (interrupt request) - sent by external devices.
 *       Can be ignored by the CPU if the status register has the INTERRUPT flag set to 1.
 *  3. RSI (reset interrupt) - resets the system.
 */
void cpu_set_interrupt(CPU *cpu, Interrupt interrupt);

#undef CPU_MEM_SIZE

#endif
