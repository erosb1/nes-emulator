/*
 * NES emulator compatible with iNES (https://www.nesdev.org/wiki/INES)
 */

#include <stdio.h>
#include <stdlib.h>

#define HEADER_SIZE 0x10
#define PRG_SIZE_HEADER_IDX 0x04
#define CHR_SIZE_HEADER_IDX 0x05

#define PRG_SIZE_UNIT 0x4000
#define CHR_SIZE_UNIT 0x2000

#define PC_OFFSET 0xC000
#define RESET_VECTOR_OFFSET 0xFFFC

#define BYTE_SIZE 0x08
#define TRUE 1
#define FALSE 0

size_t readFile(unsigned char **buffer, const char *path) {
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
size_t read_header(const unsigned char *buffer) {
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

  return HEADER_SIZE;
}

// https://www.masswerk.at/6502/6502_instruction_set.html
size_t read_prg(const unsigned char *data, const unsigned char *buffer) {

  uint16_t entrypoint = ((buffer[RESET_VECTOR_OFFSET + 1] << BYTE_SIZE) |
                         buffer[RESET_VECTOR_OFFSET]) -
                        PC_OFFSET;

  printf("Entrypoint: 0x%04hx\n", entrypoint);

  uint16_t pc = entrypoint;
  while (TRUE) {
    if (pc == 12) { // arbitrary number for testing
      break;
    }
    printf("Instruction 0x%04hx: 0x%02hx\n", pc, buffer[pc]);
    ++pc;
  }

  return data[PRG_SIZE_HEADER_IDX] * PRG_SIZE_UNIT;

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

size_t read_chr(const unsigned char *data, const unsigned char *buffer) {
  return data[CHR_SIZE_HEADER_IDX] * CHR_SIZE_UNIT;
}

int main(int argc, char *argv[]) {
  unsigned char *data;

  if (argc != 2) {
    printf("Fatal Error: No filepath provided\n");
    exit(EXIT_FAILURE);
  }

  size_t size = readFile(&data, argv[1]); // size is needed for miscellaneous
  // ROM support if we are to ever decide to support NES 2.0

  unsigned char *buffer = data;

  buffer += read_header(buffer);
  // skip trainer for now
  buffer += read_prg(data, buffer);

  buffer += read_chr(data, buffer);
  free(data);
  return EXIT_SUCCESS;
}
