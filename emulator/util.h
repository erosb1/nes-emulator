#ifndef UTIL_H
#define UTIL_H

#include "common.h"
#include "opcodes.h"

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

/**
 * A helper function to check if an instruction is illegal or not
 *
 * @return 1 if illegal, 0 if legal
 */
int is_illegal(uint8_t byte);


/**
 *  This function prints a bunch of info about the instruction that is about to be executed
 *  I.E the instruction that starts at cpu->pc
 *
 *  It follows the same notation as used for nestest.txt:
 *  https://github.com/christopherpow/nes-test-roms/blob/master/other/nestest.log
 */
void log_disassembled_instruction(const CPU *cpu);

#endif
