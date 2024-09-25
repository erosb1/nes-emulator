/*
 * NES emulator compatible with iNES games without bank switching
 * (https://www.nesdev.org/wiki/INES)
 */

#include "opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// options
#define TESTING 0xC000 // entrypoint for nestest "automation mode" (comment
// out for normal entrypoint behavior)
#define BREAKPOINT 0xC76B

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
#define STACK_OFFSET 0x0100
#define SP_START 0x00FD // only matters for tests

// status masks
#define CARRY_MASK 0x01
#define ZERO_MASK 0x02
#define IRQ_DISABLE_MASK 0x04
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
#define BYTE_HI_MASK 0xFF00
#define BYTE_LO_MASK 0x00FF
#define NIBBLE_HI_MASK 0xF0
#define NIBBLE_LO_MASK 0x0F

// timing math

// NTSC https://www.emulationonline.com/systems/nes/nes-system-timing
// https://www.nesdev.org/wiki/Cycle_reference_chart
#define NTSC_CPU_CYCLES_PER_FRAME 29780
#define NTSC_CPU_CYCLES_PER_FRAME_VBLANK 2273
#define NTSC_CPU_CYCLES_PER_FRAME_ACTIVE                                       \
  (NTSC_CPU_CYCLES_PER_FRAME - NTSC_CPU_CYCLES_PER_FRAME_VBLANK)
#define NTSC_CPU_EXTRA_ACTIVE_CYCLE 2 // TODO: should be 3 if rendering is
// disabled during the 20th scanline
#define NTSC_CPU_EXTRA_VBLANK_CYCLE 3

// PPU registers (mapped to CPU address space)

// PPUCTRL
// offset
#define PPUCTRL_OFFSET 0x2000
// masks  TODO: add the rest of these
#define NMI_ENABLE_MASK 0x80

// PPUSTATUS masks
// offset
#define PPUSTATUS_OFFSET 0x2002
// masks  TODO: add the rest of these
#define VBLANK_MASK 0x80

// cpu parameters
// #define BOOTUP_SEQUENCE_CYCLES 0x07

struct CPU {
  uint16_t pc; // program counter
  uint8_t ac;  // accumulator
  uint8_t x;   // x register
  uint8_t y;   // y register
  uint8_t sr;  // status register [NV-BDIZC]
  uint8_t sp;  // stack pointer (wraps)
  uint8_t mem[CPU_MEM_SIZE];
  size_t cur_cycle;
};

struct PPU {
  uint8_t extra_cycle_active;
  uint8_t extra_cycle_vblank;
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
         "SP:0x%02hX Cycle:%zu\t",
         cpu->pc, cpu->mem[cpu->pc], cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp,
         cpu->cur_cycle);
}

uint16_t load_2_bytes(uint8_t *mem, uint16_t offset) {
  return (mem[offset + 1] << BYTE_SIZE) | mem[offset];
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

// TODO: add all of these
void cpu_run_instruction(struct CPU *cpu) {
  uint8_t *mem = cpu->mem;
  uint8_t opcode = mem[cpu->pc];

  switch (opcode) {
  case BRK: { // not tested properly
    mem[STACK_OFFSET + cpu->sp] = cpu->pc + 2;
    mem[STACK_OFFSET + cpu->sp - 1] = cpu->sr | BREAK_MASK;
    cpu->sp -= 3;
    cpu->pc = load_2_bytes(cpu->mem, NMI_VECTOR_OFFSET);
    cpu->sr |= IRQ_DISABLE_MASK;
    cpu->cur_cycle += 7;
    printf("BRK\n");
    break;
  }

  case CLC: {
    cpu->sr ^= CARRY_MASK;
    cpu->cur_cycle += 2;
    printf("CLC\n");
    break;
  }

  case JSR: {
    mem[STACK_OFFSET + cpu->sp] = cpu->pc + 2;
    cpu->sp -= 2;
    cpu->pc += 1;
    uint16_t jump_addr = load_2_bytes(mem, cpu->pc);
    cpu->pc = jump_addr - 1;
    cpu->cur_cycle += 6;
    printf("JSR $%04hX\n", jump_addr);
    break;
  }

  case SEC: {
    cpu->sr |= CARRY_MASK;
    cpu->cur_cycle += 2;
    printf("SEC\n");
    break;
  }

  case JMP_abs: {
    cpu->pc += 1;
    uint16_t jump_addr = load_2_bytes(mem, cpu->pc);
    cpu->pc = jump_addr - 1;
    cpu->cur_cycle += 3;
    printf("JMP $%04hX\n", jump_addr);
    break;
  }

  case STX_zpg: {
    cpu->pc += 1;
    uint8_t zpg_addr = mem[cpu->pc];
    mem[zpg_addr] = cpu->x;
    cpu->cur_cycle += 3;
    printf("STX $%02hX\n", zpg_addr);
    break;
  }

  case BCC: {
    cpu->pc += 1;
    int8_t offset = mem[cpu->pc];
    uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
                                               // instruction + offset
    if (!(cpu->sr & CARRY_MASK)) {
      cpu->cur_cycle +=
          3 + ((jump_addr & BYTE_HI_MASK) == (cpu->pc & BYTE_LO_MASK)); // 4 if
      // address is on different page
      cpu->pc = jump_addr - 1;
    } else {
      cpu->cur_cycle += 2;
    }
    printf("BCC $%04hX\n", jump_addr);
    break;
  }

  case SEI: {
    cpu->sr |= IRQ_DISABLE_MASK;
    cpu->cur_cycle += 2;
    printf("SEI\n");
    break;
  }

  case CLD: {
    cpu->sr ^= DECIMAL_MASK;
    cpu->cur_cycle += 2;
    printf("CLD\n");
    break;
  }

  case LDX_imm: {
    cpu->pc += 1;
    uint8_t imm = mem[cpu->pc];
    cpu->x = imm;
    if (imm < 0) {
      cpu->sr |= NEGATIVE_MASK;
      cpu->sr ^= ZERO_MASK;
    } else if (imm == 0) {
      cpu->sr ^= NEGATIVE_MASK;
      cpu->sr |= ZERO_MASK;
    } else {
      cpu->sr ^= NEGATIVE_MASK;
      cpu->sr ^= ZERO_MASK;
    }
    cpu->cur_cycle += 2;
    printf("LDX #$%02hX\n", imm);
    break;
  }

  case LDA_imm: {
    cpu->pc += 1;
    int8_t imm = mem[cpu->pc];
    cpu->ac = imm;
    if (imm < 0) {
      cpu->sr |= NEGATIVE_MASK;
      cpu->sr ^= ZERO_MASK;
    } else if (imm == 0) {
      cpu->sr ^= NEGATIVE_MASK;
      cpu->sr |= ZERO_MASK;
    } else {
      cpu->sr ^= NEGATIVE_MASK;
      cpu->sr ^= ZERO_MASK;
    }
    cpu->cur_cycle += 2;
    printf("LDX #$%02hX\n", imm);
    break;
  }

  case BCS: {
    cpu->pc += 1;
    int8_t offset = mem[cpu->pc];
    uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
                                               // instruction + offset
    if (cpu->sr & CARRY_MASK) {
      cpu->cur_cycle +=
          3 + ((jump_addr & BYTE_HI_MASK) == (cpu->pc & BYTE_LO_MASK)); // 4 if
      // address is on different page
      cpu->pc = jump_addr - 1;
    } else {
      cpu->cur_cycle += 2;
    }
    printf("BCS $%04hX\n", jump_addr);
    break;
  }

  case NOP: {
    cpu->cur_cycle += 2;
    printf("NOP\n");
    break;
  }

  case BEQ: {
    cpu->pc += 1;
    int8_t offset = mem[cpu->pc];
    uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
                                               // instruction + offset
    if (cpu->sr & ZERO_MASK) {
      cpu->cur_cycle +=
          3 + ((jump_addr & BYTE_HI_MASK) == (cpu->pc & BYTE_LO_MASK)); // 4 if
      // address is on different page
      cpu->pc = jump_addr - 1;
    } else {
      cpu->cur_cycle += 2;
    }
    printf("BEQ $%04hX\n", jump_addr);
    break;
  }

  default:
    printf("\n");
  }
}

void cpu_run_instructions(struct CPU *cpu, struct PPU *ppu,
                          size_t cycles) { // ppu
  // only needed for logging purposes
  //
  while (cpu->cur_cycle < cycles) {
#ifdef TESTING
    if (cpu->pc == BREAKPOINT + 1) {
      break;
    }
#endif /* ifdef TESTING */

    print_state(cpu, ppu);
    cpu_run_instruction(cpu);

    ++cpu->pc;
  }
  cpu->cur_cycle = 0; // to prevent overflow
}

void new_frame(struct CPU *cpu, struct PPU *ppu) {

  // maybe TODO: implement true cycle accurate timing (draw scanlines instead
  // of frames)

  ppu_vblank_set(cpu->mem, FALSE);
  cpu_run_instructions(
      cpu, ppu, NTSC_CPU_CYCLES_PER_FRAME_ACTIVE + ppu->extra_cycle_active);

  // TODO: draw frame

  ppu_vblank_set(cpu->mem, TRUE);
  ppu_maybe_nmi(cpu);

  cpu_run_instructions(
      cpu, ppu, NTSC_CPU_CYCLES_PER_FRAME_VBLANK + ppu->extra_cycle_active);
}

// https://www.masswerk.at/6502/6502_instruction_set.html
void run_prg(struct CPU *cpu, struct PPU *ppu) {
  printf("Execution: (");

  uint16_t entrypoint = load_2_bytes(cpu->mem, RESET_VECTOR_OFFSET);

#ifdef TESTING
  entrypoint = TESTING;
  printf("breakpoint: 0x%04hX, ", BREAKPOINT);
#endif /* ifdef TESTING */

  printf("entrypoint: 0x%04hX)\n", entrypoint);

  cpu->pc = entrypoint;

  ppu->extra_cycle_active = 0;
  ppu->extra_cycle_vblank = 0;
  for (size_t frame = 0; TRUE; ++frame) {
    if (frame == NTSC_CPU_EXTRA_ACTIVE_CYCLE * NTSC_CPU_EXTRA_VBLANK_CYCLE) {
      frame = 0; // to prevent overflow
    }
    if (frame % NTSC_CPU_EXTRA_ACTIVE_CYCLE == 0) {
      ppu->extra_cycle_active = 1;
    }
    if (frame % NTSC_CPU_EXTRA_VBLANK_CYCLE == 0) {
      ppu->extra_cycle_vblank = 1;
    }
    new_frame(cpu, ppu);
  }
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

  struct CPU cpu = {.sp = SP_START, .cur_cycle = 7};
  struct PPU ppu = {}; // partially initialize to zero all fields

  read_header_debug(buffer);
  static_memmap(buffer, cpu.mem, ppu.mem);
  free(buffer);

  // skip trainer for now
  run_prg(&cpu, &ppu);

  return EXIT_SUCCESS;
}
