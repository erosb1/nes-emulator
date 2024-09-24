/*
 * NES emulator compatible with iNES (https://www.nesdev.org/wiki/INES)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// options
#define INSTRUCTION_COUNT 12 // amount of instructions to run

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

// vectors/offsets
#define RESET_VECTOR_OFFSET 0xFFFC

// mappers
// nrom
// cpu
#define NROM_PRG_OFFSET_1 0x8000
#define NROM_PRG_OFFSET_2 0xC000
// ppu
#define NROM_CHR_OFFSET 0x0000

// stack parameters
#define STACK_SIZE 0x0100

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

struct CPU {
  uint16_t pc; // program counter
  uint8_t ac;  // accumulator
  uint8_t x;   // x register
  uint8_t y;   // y register
  uint8_t s;   // status register [NV-BDIZC]
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
    printf("0x%02hx", buffer[i]);
    printf("(%c) ", buffer[i]);
  }
  printf("\n");

  printf("PRG-ROM size (16kb units):\n");
  printf("0x%02hx", buffer[PRG_SIZE_HEADER_IDX]);
  printf("\n");

  printf("CHR-ROM size (8kb units):\n");
  printf("0x%02hx", buffer[CHR_SIZE_HEADER_IDX]);
  printf("\n");

  printf("Flags 6:\n");
  printf("0x%02hx", buffer[6]);
  printf("\n");

  printf("Flags 7:\n");
  printf("0x%02hx", buffer[7]);
  printf("\n");
  printf("\n");
}

void cpu_run_instruction(struct CPU *cpu) {
  printf("Instruction 0x%04hx: 0x%02hx\n", cpu->pc, cpu->mem[cpu->pc]);
}

// https://www.masswerk.at/6502/6502_instruction_set.html
void run_prg(struct CPU *cpu, struct PPU *ppu) {
  printf("Execution:\n");

  uint16_t entrypoint = (cpu->mem[RESET_VECTOR_OFFSET + 1] << BYTE_SIZE) |
                        cpu->mem[RESET_VECTOR_OFFSET];

  printf("Entrypoint: 0x%04hx\n", entrypoint);

  cpu->pc = entrypoint;
  cpu->sp = 0x00; // may or may not be needed

  while (TRUE) {
    if (cpu->pc == entrypoint + INSTRUCTION_COUNT) {
      break;
    }
    cpu_run_instruction(cpu);
    ++cpu->pc;
  }

  // uint64_t src = 1;
  // uint64_t dst;
  //
  // __asm__("\
  // mov %0, %1\n\t\
  // add %0, %0, %1\n\t"
  //         : "=r"(dst)
  //         : "r"(src)); // GCC
  //
  // printf("0x%llu\n", dst);
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

  struct CPU cpu;
  struct PPU ppu;

  read_header_debug(buffer);
  static_memmap(buffer, cpu.mem, ppu.mem);

  // skip trainer for now
  run_prg(&cpu, &ppu);

  free(buffer);
  return EXIT_SUCCESS;
}
