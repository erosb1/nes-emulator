#include "timer.h"

#ifdef RISC_V

uint32_t get_time_point() {
    // TODO: implement based on DTEKV timer
    return 0;
}

uint32_t get_elapsed_us(uint32_t time_point_start, uint32_t time_point_end) {
    // TODO: implement based on DTEKV timer
    return time_point_end - time_point_start;
}

void sleep_us(uint32_t microseconds) {
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

#include <SDL.h>

uint32_t get_time_point() { return (uint32_t)SDL_GetPerformanceCounter(); }

uint32_t get_elapsed_us(uint32_t time_point_start, uint32_t time_point_end) {
    double elapsed = time_point_end - time_point_start;
    double frequency = SDL_GetPerformanceFrequency();
    double elapsed_microseconds = (elapsed / frequency) * 1e6;
    return (uint32_t) elapsed_microseconds;
}

void sleep_us(uint32_t microseconds) {
    if (microseconds >= 1000) {
        uint32_t milliseconds = microseconds / 1000;
        SDL_Delay(milliseconds);
        microseconds %= 1000;
    }

    if (microseconds > 0) {
        uint64_t start = SDL_GetPerformanceCounter();
        uint64_t frequency = SDL_GetPerformanceFrequency();
        uint64_t ticks = (microseconds * frequency) / 1e6;

        while ((SDL_GetPerformanceCounter() - start) < ticks) {
            SDL_Delay(0);
        }
    }
}

#endif