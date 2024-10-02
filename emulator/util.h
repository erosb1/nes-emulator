#ifndef UTIL_H
#define UTIL_H

#include "common.h"

// forward declarations
typedef struct CPU CPU;

// utility macros
#define BYTE_SIZE 0x08
#define NIBBLE_SIZE 0x04
#define TRUE 0x1
#define FALSE 0x0
#define BYTE_HI_MASK 0xFF00
#define BYTE_LO_MASK 0x00FF
#define NIBBLE_HI_MASK 0xF0
#define NIBBLE_LO_MASK 0x0F

void print_state(CPU *cpu);

#endif
