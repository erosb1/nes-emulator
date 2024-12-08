#include "timer.h"

#ifdef RISC_V

uint32_t get_time_point() {
    uint32_t cycles;
    asm volatile("csrr %0, mcycle" : "=r"(cycles));
    return cycles;
}

uint32_t get_elapsed_us(uint32_t time_point_start, uint32_t time_point_end) {
    return (time_point_end - time_point_start) / 30;
}

void sleep_us(uint32_t microseconds) {
    uint32_t start = get_time_point();

    while (1) {
        uint32_t cur = get_time_point();
        if (get_elapsed_us(start, cur) >= microseconds)
            return;
    }
}

#else

#include <SDL.h>

uint32_t get_time_point() { return (uint32_t)SDL_GetPerformanceCounter(); }

uint32_t get_elapsed_us(uint32_t time_point_start, uint32_t time_point_end) {
    double elapsed = time_point_end - time_point_start;
    double frequency = SDL_GetPerformanceFrequency();
    double elapsed_microseconds = (elapsed / frequency) * 1e6;
    return (uint32_t)elapsed_microseconds;
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