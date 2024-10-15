#ifndef LOAD_ROM_H
#define LOAD_ROM_H

#include "common.h"


typedef enum {
  NES_UNKNOWN,
  NES_INES,
  NES_NES20,
  NES_UNIF,
  NES_FDS
} NESType;



typedef struct {
  char magic[4];
  uint8_t prg_rom_size;
  uint8_t chr_rom_size;
  uint8_t flags_6;
  uint8_t flags_7;
  uint8_t prg_ram_size;
  uint8_t flags_9;
  uint8_t flags_10;
  uint8_t unused[5];
} iNES_Header;


// forward declarations
typedef struct CPUMemory CPUMemory;
typedef struct PPUMemory PPUMemory;


/*
 * Loads the ROM in the file specified by `path`
 * Stores it as a byte array in `buffer`
 */
size_t load_rom(uint8_t **buffer, const char *path);


iNES_Header read_iNES_header(const uint8_t *buffer);
NESType detect_nes_file_type(const uint8_t *buffer);
void static_memmap(uint8_t *buffer, CPUMemory *cpu_mem, PPUMemory *ppu_mem);

#endif
