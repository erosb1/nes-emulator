#ifndef LOAD_ROM_H
#define LOAD_ROM_H

#include "common.h"

size_t load_rom(uint8_t **buffer, const char *path);
void read_header_debug(const uint8_t *buffer);
void static_memmap(uint8_t *buffer, uint8_t *cpu_mem, uint8_t *ppu_mem);

#endif
