#ifndef TIMER_H
#define TIMER_H

#include "common.h"

/**
 *  Gets a specific point in time.
 *
 */
uint32_t get_time_point();

/**
 *  Returns the amount of microseconds between two time points.
 *
 */
uint32_t get_elapsed_us(uint32_t time_point_start, uint32_t time_point_end);

/**
 *  Sleeps for a specific amount of microseconds, i.e. pauses the emulator.
 *
 */
void sleep_us(uint32_t microseconds);

#endif