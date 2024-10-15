#include "common.h"

SDLInstance SDL_INSTANCE;

#ifdef RISC_V
void sleep_us(size_t microseconds) {
    const uint8_t cycles = microseconds * 30;
    volatile uint32_t *status = (uint32_t *)0x04000020;
    *status = 0; // clear timeout

    volatile uint32_t *ctrl = (uint32_t *)(status + 4);
    volatile uint32_t *periodl = (uint32_t *)(status + 8);
    volatile uint32_t *periodh = (uint32_t *)(status + 16);
    *periodl = cycles & 0xFFFF;
    *periodh = cycles >> 16;

    // STOP = 0; START = 1; CONT = 1; ITO = 0.
    *ctrl = 0b0100;

    while (1) {
        if (*status & 0x1) { // if timeout
            return;
        }
    }
}

#else

void sleep_us(unsigned long microseconds) {
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000UL;           // whole seconds
    ts.tv_nsec = (microseconds % 1000000UL) * 1000; // remainder, in nanoseconds
    nanosleep(&ts, NULL);
}

#endif // RISC_V
