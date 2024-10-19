#ifndef DEBUG_H
#define DEBUG_H

#include "sdl-instance.h"

// forward declarations
typedef struct Emulator Emulator;
typedef struct CPU CPU;

/**
 *  This function prints a bunch of info about the instruction that is about to
 *  be executed I.E the instruction that starts at cpu->pc
 *
 *  It follows the same notation as used for nestest.txt:
 *  https://github.com/christopherpow/nes-test-roms/blob/master/other/nestest.log
 */
void debug_log_instruction(CPU *cpu);

void debug_draw_screen(const Emulator *emulator);

#endif