/*
 * NES emulator compatible with iNES games without bank switching
 * (https://www.nesdev.org/wiki/INES)
 */

#include "opcodes.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// options
#define TESTING 0xC000 // entrypoint for nestest "automation mode" (comment
// out for normal entrypoint behavior)
#define BREAKPOINT 0xC00C

// map_mem parameters
#define CPU_MEM_SIZE 0x10000 // 64KiB
#define PPU_MEM_SIZE 0x4000  // 16KiB

// header parameters
#define HEADER_SIZE 0x10
#define PRG_SIZE_HEADER_IDX 0x04
#define CHR_SIZE_HEADER_IDX 0x05
#define FLAGS_6_HEADER_IDX 0x06
#define FLAGS_7_HEADER_IDX 0x07

// size units
#define PRG_SIZE_UNIT 0x4000 // 16 KiB
#define CHR_SIZE_UNIT 0x2000 // 8 KiB

// vector offsets
#define RESET_VECTOR_OFFSET 0xFFFC
#define NMI_VECTOR_OFFSET 0xFFFA

// mappers
// nrom
// cpu
#define NROM_PRG_OFFSET_1 0x8000
#define NROM_PRG_OFFSET_2 0xC000
// ppu
#define NROM_CHR_OFFSET 0x0000

// stack parameters
#define STACK_SIZE 0x0100
#define STACK_OFFSET 0x0100
#define SP_START 0x00FF

// status masks
#define CARRY_MASK 0x01
#define ZERO_MASK 0x02
#define INTERRUPT_MASK 0x04
#define DECIMAL_MASK 0x08
#define BREAK_MASK 0x10
// bit 5 is ignored
#define OVERFLOW_MASK 0x40
#define NEGATIVE_MASK 0x80

// utility macros
#define BYTE_SIZE 0x08
#define NIBBLE_SIZE 0x04
#define TRUE 0x1
#define FALSE 0x0
#define NIBBLE_HI_MASK 0xF0
#define NIBBLE_LO_MASK 0x0F

// timing math NTSC
// #define NTSC_CPU_FREQ 1789773 // 1.789 Mhz
// #define NTSC_FRAME_RATE 60

// PPU registers (mapped to CPU address space)

// PPUCTRL
// offset
#define PPUCTRL_OFFSET 0x2000
// masks  TODO: Add the rest of these
#define NMI_ENABLE_MASK 0x80

// PPUSTATUS masks
// offset
#define PPUSTATUS_OFFSET 0x2002
// masks  TODO: Add the rest of these
#define VBLANK_MASK 0x80

struct CPU {
  uint16_t pc; // program counter
  uint8_t ac;  // accumulator
  uint8_t x;   // x register
  uint8_t y;   // y register
  uint8_t sr;  // status register [NV-BDIZC]
  uint8_t sp;  // stack pointer (wraps)
  uint8_t mem[CPU_MEM_SIZE];
};

struct PPU {
  uint8_t mem[PPU_MEM_SIZE];
};

size_t load_rom(uint8_t **buffer, const char *path) {
  FILE *fp;
  size_t expected_size;
  size_t actual_size;

  fp = fopen(path, "rb");
  if (fp == NULL) {
    printf("Fatal Error: Failed to open file %s\n", path);
    exit(EXIT_FAILURE);
  }

  fseek(fp, 0, SEEK_END); // seek to end of file
  expected_size = ftell(fp);
  fseek(fp, 0, SEEK_SET); // seek to start of file
  *buffer = malloc(expected_size);

  actual_size = fread(*buffer, sizeof **buffer, expected_size, fp); // read
  fclose(fp);

  if (actual_size != expected_size) {
    printf("Fatal Error: Expected size (%zu) != actual size (%zu) %s\n",
           expected_size, actual_size, path);
    exit(EXIT_FAILURE);
  }
  return actual_size;
}

// https://www.nesdev.org/wiki/INES#iNES_file_format
void read_header_debug(const uint8_t *buffer) {
  printf("File Identifier:\n");
  for (int i = 0; i < 4; ++i) {
    printf("0x%02hX", buffer[i]);
    printf("(%c) ", buffer[i]);
  }
  printf("\n");

  printf("PRG-ROM size (16kb units):\n");
  printf("0x%02hX", buffer[PRG_SIZE_HEADER_IDX]);
  printf("\n");

  printf("CHR-ROM size (8kb units):\n");
  printf("0x%02hX", buffer[CHR_SIZE_HEADER_IDX]);
  printf("\n");

  printf("Flags 6:\n");
  printf("0x%02hX", buffer[6]);
  printf("\n");

  printf("Flags 7:\n");
  printf("0x%02hX", buffer[7]);
  printf("\n");
  printf("\n");
}

void print_state(struct CPU *cpu, struct PPU *ppu) {
  printf("PC:0x%04hX Opcode:0x%02hX AC:0x%02hX X:0x%02hX Y:0x%02hX SR:0x%02hX "
         "SP:0x%02hX\n",
         cpu->pc, cpu->mem[cpu->pc], cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp);
}

uint16_t load_2_bytes(uint8_t *mem, uint16_t offset) {
  return (mem[offset + 1] << BYTE_SIZE) | mem[offset];
}

void cpu_run_instruction(struct CPU *cpu) {
  uint8_t opcode = cpu->mem[cpu->pc];
  uint8_t *mem = cpu->mem;

  switch (opcode) {
  case BRK: { // not tested
    cpu->mem[cpu->sp] = cpu->pc + 2;
    cpu->mem[cpu->sp - 1] = cpu->sr | BREAK_MASK;
    cpu->sp -= 3;
    cpu->pc = load_2_bytes(cpu->mem, NMI_VECTOR_OFFSET);
    cpu->sr |= INTERRUPT_MASK;
    return;
  }
  case SEI: {
    cpu->sr |= INTERRUPT_MASK;
    return;
  }
  case CLD: {
    cpu->sr ^= DECIMAL_MASK;
    return;
  }
  case LDX_imm: {
    ++cpu->pc;
    cpu->x = cpu->pc;
    return;
  }
  }
}

void ppu_vblank_set(uint8_t *cpu_mem, uint8_t bool) {
  if (bool) {
    cpu_mem[PPUCTRL_OFFSET] |= VBLANK_MASK;
  } else {
    cpu_mem[PPUCTRL_OFFSET] ^= VBLANK_MASK;
  }
}

void ppu_maybe_nmi(struct CPU *cpu) {
  if (cpu->mem[PPUSTATUS_OFFSET] & NMI_ENABLE_MASK) {
    cpu->pc = load_2_bytes(cpu->mem, NMI_VECTOR_OFFSET);
  }
}

void new_frame(struct CPU *cpu, struct PPU *ppu) {

  ppu_vblank_set(cpu->mem, FALSE);
  while (TRUE) {
#ifdef TESTING
    if (cpu->pc == BREAKPOINT) {
      break;
    }
#endif /* ifdef TESTING */

    print_state(cpu, ppu);
    cpu_run_instruction(cpu);
    ++cpu->pc;
  }

  // TODO: draw frame

  ppu_vblank_set(cpu->mem, TRUE);

  while (TRUE) {
#ifdef BREAKPOINT
    if (cpu->pc == BREAKPOINT) {
      break;
    }
#endif /* ifdef BREAKPOINT */

    print_state(cpu, ppu);
    cpu_run_instruction(cpu);
    ++cpu->pc;
  }
  ppu_maybe_nmi(cpu);
}

// https://www.masswerk.at/6502/6502_instruction_set.html
void run_prg(struct CPU *cpu, struct PPU *ppu) {
  printf("Execution:\n");

  uint16_t entrypoint = load_2_bytes(cpu->mem, RESET_VECTOR_OFFSET);

#ifdef TESTING
  entrypoint = TESTING;

#endif /* ifdef TESTING */

  printf("Entrypoint: 0x%04hX\n", entrypoint);

  cpu->pc = entrypoint;

  new_frame(cpu, ppu);
}

void static_memmap(uint8_t *buffer, uint8_t *cpu_mem, uint8_t *ppu_mem) {
  size_t prg_size = buffer[PRG_SIZE_HEADER_IDX];
  size_t prg_size_bytes = prg_size * PRG_SIZE_UNIT;
  uint8_t chr_size = buffer[CHR_SIZE_HEADER_IDX];
  uint8_t chr_size_bytes = chr_size * CHR_SIZE_UNIT;

  uint8_t mapper_num = (buffer[FLAGS_6_HEADER_IDX] >> NIBBLE_SIZE) |
                       (buffer[FLAGS_7_HEADER_IDX] & NIBBLE_HI_MASK);

  buffer += HEADER_SIZE;

  switch (mapper_num) {
  case 0: {
    // #000 (nrom)
    memcpy(cpu_mem + NROM_PRG_OFFSET_1, buffer, prg_size_bytes);
    memcpy(ppu_mem + NROM_CHR_OFFSET, buffer + prg_size_bytes, chr_size_bytes);
    if (prg_size == 1) {
      memcpy(cpu_mem + NROM_PRG_OFFSET_2, buffer, prg_size_bytes);
      break;
    }
    if (prg_size == 2) {
      memcpy(cpu_mem + NROM_PRG_OFFSET_2, buffer + PRG_SIZE_UNIT,
             prg_size_bytes);
      break;
    }
  }
  default: {
    printf("Fatal Error: Bank switching not yet implemented\n");
    exit(EXIT_FAILURE);
  }
  }
}

int main(int argc, char *argv[]) {
  uint8_t *buffer;

  if (argc != 2) {
    printf("Fatal Error: No filepath provided\n");
    exit(EXIT_FAILURE);
  }

  size_t size = load_rom(&buffer, argv[1]); // size is needed to calculate the
  // misc roms section size for NES 2.0

  struct CPU cpu = {.sp = SP_START};
  struct PPU ppu = {};

  read_header_debug(buffer);
  static_memmap(buffer, cpu.mem, ppu.mem);

  // skip trainer for now
  run_prg(&cpu, &ppu);

  free(buffer);
  return EXIT_SUCCESS;
}
