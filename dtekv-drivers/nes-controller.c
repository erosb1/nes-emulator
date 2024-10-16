#include "nes-controller.h"

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

typedef enum Direction { OUTPUT = 1, INPUT = 0 } Direction;

uint8_t get_pin(uint32_t pin) { return (*(volatile int32_t *)GPIO_DATA & pin) != 0; }

void set_pin(uint32_t pin, uint32_t state) {
    volatile uint32_t *reg = (uint32_t *)GPIO_DATA;
    if (state) {
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

void input_clock_pulse() {
    set_pin(CLOCK_PIN_MASK, HIGH);
    sleep_us(SHIFT_DELAY); // may or may not be needed depending on the clock speed
    set_pin(CLOCK_PIN_MASK, LOW);
}

void setup_input() {
    // configure pin directions
    set_pin_direction(DATA_PIN_MASK, INPUT);
    set_pin_direction(CLOCK_PIN_MASK, OUTPUT);
    set_pin_direction(LATCH_PIN_MASK, OUTPUT);

    // control lines idle low
    set_pin(CLOCK_PIN_MASK, LOW);
    set_pin(LATCH_PIN_MASK, LOW);
}

uint8_t poll_input() {
    // // NES controller button states are asynchronously loaded into the 4021
    // // while latch is high
    //
    // // when the latch pin goes low, the first data bit is
    // // shifted to data
    //
    // // button data is shifted to the data pin on each low to high
    // // transition of the clock pin
    //
    // // latch pulse
    // pulse(LATCH_PIN_MASK);
    //
    // uint8_t controller_state = 0;
    // for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {         // read in the 8 controller buttons that
    //                                                      // were latched into the 4021
    //     controller_state |= get_pin(DATA_PIN_MASK) << i; // store the current button state on the data input
    //
    //     // generate a clock pulse to bring the
    //     // next button to the data input
    //     pulse(CLOCK_PIN_MASK);
    // }
    //
    // // order bit0->bit7 = a, b, select, start, up, down, left, right
    // return ~controller_state;
    return 0;
}
