#ifndef DEBUG_H
#define DEBUG_H


// forward declarations
typedef struct CPU CPU;


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