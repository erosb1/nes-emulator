#ifndef CPU_H
#define CPU_H

#include "common.h"
#include "cpu_memory.h"


typedef struct CPU {
  uint16_t pc; // program counter
  uint8_t ac;  // accumulator
  uint8_t x;   // x register
  uint8_t y;   // y register
  uint8_t sr;  // status register [NV-BDIZC]
  uint8_t sp;  // stack pointer (wraps)
  CPUMemory* mem;
  size_t cur_cycle;
} CPU;

void init_cpu(CPU *cpu, CPUMemory *mem);
void ppu_vblank_set(uint8_t *cpu_mem, uint8_t bool);
void ppu_maybe_nmi(CPU *cpu);

void cpu_run_instruction(CPU *cpu);
void cpu_run_instructions(CPU *cpu, size_t cycles);

#undef CPU_MEM_SIZE

#endif
