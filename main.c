/*
 * NES emulator compatible with iNES (https://www.nesdev.org/wiki/INES)
 */

#include <stdio.h>
#include <stdlib.h>

#define HEADER_SIZE 16
#define PRG_ROM_SIZE_UNIT 16E3

size_t readFile(char **buffer, char *path) {
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

void read_program(char *buffer) {

  // size_t size = buffer[4] * PRG_ROM_SIZE_UNIT;
  size_t size = 12;
  buffer += HEADER_SIZE;

  for (int i = 0; i < size; ++i) {
    printf("Instruction %d: 0x%02hhx\n", i, buffer[i]);
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

// https://www.nesdev.org/wiki/INES#iNES_file_format
void read_header(char *buffer) {
  printf("File Identifier:\n");
  for (int i = 0; i < 4; ++i) {
    printf("0x%02hhx", buffer[i]);
    printf("(%c) ", buffer[i]);
  }
  printf("\n");

  printf("PRG-ROM size LSB (16kb units):\n");
  printf("0x%02hhx", buffer[4]);
  printf("\n");

  printf("CHR-ROM size LSB (8kb units):\n");
  printf("0x%02hhx", buffer[5]);
  printf("\n");

  printf("Flags 6:\n");
  printf("0x%02hhx", buffer[6]);
  printf("\n");

  printf("Flags 7:\n");
  printf("0x%02hhx", buffer[7]);
  printf("\n");
}

int main(int argc, char *argv[]) {
  char *buffer;

  if (argc != 2) {
    printf("Fatal Error: No filepath provided\n");
    exit(EXIT_FAILURE);
  }

  size_t size = readFile(&buffer, argv[1]); // size is needed for miscellaneous
  // ROM support if we are to ever decide to support NES 2.0

  read_header(buffer);
  // skip trainer for now
  read_program(buffer);
  // emulate(buffer, size);

  free(buffer);
  return EXIT_SUCCESS;
}
