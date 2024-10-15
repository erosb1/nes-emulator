#ifndef CPU_H
#define CPU_H

#include "common.h"

// forward declarations
typedef struct CPUMemory CPUMemory;

// enum shorthand for toggling CPU flags
typedef enum CPUFlag {
    CARRY_MASK = (1 << 0),
    ZERO_MASK = (1 << 1),
    INTERRUPT_MASK = (1 << 2),
    DECIMAL_MASK = (1 << 3),
    BREAK_MASK = (1 << 4),
    UNUSED_MASK = (1 << 5),
    OVERFLOW_MASK = (1 << 6),
    NEGATIVE_MASK = (1 << 7),
} CPUFlag;

typedef struct CPU {
    uint16_t pc; // program counter
    uint16_t address; // addressing mode address (set before executing each instruction)
    uint8_t ac;  // accumulator
    uint8_t x;   // x register
    uint8_t y;   // y register
    uint8_t sr;  // status register [NV-BDIZC]
    uint8_t sp;  // stack pointer (wraps)
    CPUMemory *mem;
    size_t cur_cycle;

    int is_logging;
} CPU;

void init_cpu(CPU *cpu);

void cpu_run_instruction(CPU *cpu);

#undef CPU_MEM_SIZE

#endif
