#ifndef TIMER_H
#define TIMER_H

#include "common.h"


uint32_t get_time_point();
uint32_t get_elapsed_us(uint32_t time_point_start, uint32_t time_point_end);
void sleep_us(uint32_t microseconds);




#endif