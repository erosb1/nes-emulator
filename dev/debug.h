#ifndef DEBUG_H
#define DEBUG_H

#include "sdl-instance.h"

// forward declarations
typedef struct Emulator Emulator;
typedef struct CPU CPU;
typedef struct MEM MEM;

/**
 *  Prints a bunch of info about the instruction that is about to
 *  be executed, i.e. the instruction that starts at cpu->pc
 *
 *  It follows the same notation as used for nestest.txt:
 *  https://github.com/christopherpow/nes-test-roms/blob/master/other/nestest.log
 */
void debug_log_instruction(const CPU *cpu);

/**
 *  Dumps a region of cpu memory to the console.
 *
 */
void debug_memory_dump(const MEM *mem, uint16_t start, uint16_t len);

/**
 *  Dumps a region of cpu memory to the console, but in ascii format.
 *
 */
void debug_memory_dump_ascii(const MEM *mem, uint16_t start, uint16_t len);

/**
 *  Draws the DEBUG_SCREEN window region to the SDL window.
 *
 */
void debug_draw_screen(const Emulator *emulator);

/**
 *  Freezes the frame, so that you can inspect the rendering at that point.
 *
 */
void debug_pause_screen(Emulator *emulator);

#endif