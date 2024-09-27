#ifndef CPU_H
#define CPU_H

#include "common.h"

// options
#define TESTING 0xC000 // entrypoint for nestest "automation mode" (comment
// out for normal entrypoint behavior)
#define BREAKPOINT 0xC783 // uncomment to run normally

#define CPU_MEM_SIZE 0x10000 // 64KiB

typedef struct CPU {
    uint16_t pc; // program counter
    uint8_t ac; // accumulator
    uint8_t x; // x register
    uint8_t y; // y register
    uint8_t sr; // status register [NV-BDIZC]
    uint8_t sp; // stack pointer (wraps)
    uint8_t mem[CPU_MEM_SIZE];
    size_t cur_cycle;
} CPU;

void ppu_vblank_set(uint8_t *cpu_mem, uint8_t bool);
void ppu_maybe_nmi(CPU *cpu);

void cpu_run_instruction(CPU *cpu);
void cpu_run_instructions(CPU *cpu, size_t cycles);

#undef CPU_MEM_SIZE

#endif
