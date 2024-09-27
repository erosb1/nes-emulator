#include "load_rom.h"
#include "console.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

// size units
#define PRG_SIZE_UNIT 0x4000 // 16 KiB
#define CHR_SIZE_UNIT 0x2000 // 8 KiB

// header parameters
#define HEADER_SIZE 0x10
#define PRG_SIZE_HEADER_IDX 0x04
#define CHR_SIZE_HEADER_IDX 0x05
#define FLAGS_6_HEADER_IDX 0x06
#define FLAGS_7_HEADER_IDX 0x07

// mappers
// nrom
// cpu
#define NROM_PRG_OFFSET_1 0x8000
#define NROM_PRG_OFFSET_2 0xC000
// ppu
#define NROM_CHR_OFFSET 0x0000

size_t load_rom(uint8_t **buffer, const char *path) {
  FILE *fp;
  size_t expected_size;
  size_t actual_size;

  fp = fopen(path, "rb");
  if (fp == NULL) {
    print("Fatal Error: Failed to open file %s\n", path);
    exit(EXIT_FAILURE);
  }

  fseek(fp, 0, SEEK_END); // seek to end of file
  expected_size = ftell(fp);
  fseek(fp, 0, SEEK_SET); // seek to start of file
  *buffer = malloc(expected_size);

  actual_size = fread(*buffer, sizeof **buffer, expected_size, fp); // read
  fclose(fp);

  if (actual_size != expected_size) {
    print("Fatal Error: Expected size (%zu) != actual size (%zu) %s\n",
          expected_size, actual_size, path);
    exit(EXIT_FAILURE);
  }
  return actual_size;
}

// https://www.nesdev.org/wiki/INES#iNES_file_format
void read_header_debug(const uint8_t *buffer) {
  print("File Identifier:\n");
  for (int i = 0; i < 4; ++i) {
    print("0x%02hX", buffer[i]);
    print("(%c) ", buffer[i]);
  }
  print("\n");

  print("PRG-ROM size (16kb units):\n");
  print("0x%02hX", buffer[PRG_SIZE_HEADER_IDX]);
  print("\n");

  print("CHR-ROM size (8kb units):\n");
  print("0x%02hX", buffer[CHR_SIZE_HEADER_IDX]);
  print("\n");

  print("Flags 6:\n");
  print("0x%02hX", buffer[6]);
  print("\n");

  print("Flags 7:\n");
  print("0x%02hX", buffer[7]);
  print("\n");
  print("\n");
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
    print("Fatal Error: Bank switching not yet implemented\n");
    exit(EXIT_FAILURE);
  }
  }
}
