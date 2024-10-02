#ifndef LOAD_ROM_H
#define LOAD_ROM_H

#include "common.h"

// forward declarations
typedef struct CPUMemory CPUMemory;
typedef struct PPUMemory PPUMemory;

size_t load_rom(uint8_t **buffer, const char *path);
void read_header_debug(const uint8_t *buffer);
void static_memmap(uint8_t *buffer, CPUMemory *cpu_mem, PPUMemory *ppu_mem);

#endif
