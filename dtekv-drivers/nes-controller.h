#ifndef NES_CONTROLLER_H
#define NES_CONTROLLER_H

#include "common.h"

// Driver for the NES controller (this file is only run on the DTEK-V board)
void setup_input();
uint8_t poll_input();

#endif // NES_CONTROLLER_H
