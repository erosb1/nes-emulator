#include "nes-controller.h"
#include "common.h"
#include "timer.h"

// reference:
// https://github.com/GadgetReboot/Arduino/blob/master/Teensy/NES_To_USB/NES_To_USB.ino

/*
  Controller Button States:
  nes_register [7..0] = [Right, Left, Down, Up, Start, Select, B, A]

  NES controller pinout:
                   _________
                 /          |
                /        O  | Ground
               /            |
  +VCC (3.3V)  |  O      O  | Clock
               |            |
         N.C.  |  O      O  | Latch
               |            |
         N.C.  |  O      O  | Data Out (To DTEK-V)
               |____________|


Corresponding DTEK-V pins:
                ____________
               |            |
  +VCC (3.3V)  |  O      O  | Ground
               |            |
   Clock [26]  |  O      O  | Latch [27]
               |            |
 Data In [28]  |  O      O  | N.C.
               |____________|

*/

// bit positions of each controller button in the status register

#define GPIO_DATA 0x40000E0
#define GPIO_DIR 0x40000E4
#define BUTTON_COUNT 8

// delay in microseconds to help with shift register setup/hold timing
#define SHIFT_DELAY 20
#define LATCH_DELAY 20

typedef enum ControllerPin {
    CLOCK_PIN = (1 << 26),
    LATCH_PIN = (1 << 27),
    DATA_PIN = (1 << 28),

} ControllerPin;

typedef enum State { HIGH = 1, LOW = 0 } State;

typedef enum Direction { OUTPUT = 1, INPUT = 0 } Direction;

uint8_t get_pin(uint32_t pin) { return (*(volatile int32_t *)GPIO_DATA & pin) != 0; }

void set_pin(uint32_t pin, State state) {
    volatile uint32_t *reg = (uint32_t *)GPIO_DATA;
    if (state == HIGH) {
        *reg |= pin;
    } else {
        *reg &= ~pin;
    }
}

void set_pin_direction(uint32_t pin, Direction dir) {
    volatile uint32_t *reg = (uint32_t *)GPIO_DIR;
    if (dir == OUTPUT) {
        *reg |= pin;
    } else {
        *reg &= ~pin;
    }
}

void pulse(uint32_t pin, uint32_t delay) {
    set_pin(pin, HIGH);
    sleep_us(delay); // may or may not be needed depending on the clock speed
    set_pin(pin, LOW);
}

uint8_t input_read() {
    pulse(CLOCK_PIN, SHIFT_DELAY);
    return get_pin(DATA_PIN);
}

void input_latch() { pulse(LATCH_PIN, LATCH_DELAY); }

void input_setup() {
    // configure pin directions
    set_pin_direction(DATA_PIN, INPUT);
    set_pin_direction(CLOCK_PIN, OUTPUT);
    set_pin_direction(LATCH_PIN, OUTPUT);

    // control lines idle low
    set_pin(CLOCK_PIN, LOW);
    set_pin(LATCH_PIN, LOW);
}
