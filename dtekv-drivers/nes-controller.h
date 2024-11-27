#ifndef NES_CONTROLLER_H
#define NES_CONTROLLER_H

#include "common.h"

// Driver for the NES controller (this file is only run on the DTEK-V board)

uint8_t input_read();
void input_latch();
void input_setup();

#endif // NES_CONTROLLER_H
