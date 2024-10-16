#ifndef NES_CONTROLLER_H
#define NES_CONTROLLER_H

#include "common.h"

// Driver for the NES controller (this file is only run on the DTEK-V board)

typedef enum ControllerPin {
    CLOCK_PIN_MASK = (1 << 26),
    LATCH_PIN_MASK = (1 << 27),
    DATA_PIN_MASK = (1 << 28),

} ControllerPin;

typedef enum State { HIGH = 1, LOW = 0 } State;

void setup_input();
uint8_t poll_input();

void input_clock_pulse();
uint8_t get_pin(uint32_t pin);
void set_pin(uint32_t pin, uint32_t state);

#endif // NES_CONTROLLER_H
