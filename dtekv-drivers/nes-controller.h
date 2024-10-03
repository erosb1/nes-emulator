#ifndef NES_CONTROLLER_H
#define NES_CONTROLLER_H

#include "common.h"

// Driver for the NES controller (this file is only run on the DTEK-V board)

typedef enum Button {
    A_BUTTON_MASK = (1 << 0),
    B_BUTTON_MASK = (1 << 1),
    SELECT_BUTTON_MASK = (1 << 2),
    START_BUTTON_MASK = (1 << 3),
    UP_BUTTON_MASK = (1 << 4),
    DOWN_BUTTON_MASK = (1 << 5),
    LEFT_BUTTON_MASK = (1 << 6),
    RIGHT_BUTTON_MASK = (1 << 7),
} Button;

void setup_input();
uint8_t poll_input();

#endif // NES_CONTROLLER_H
