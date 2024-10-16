#ifndef COMMON_H
#define COMMON_H

#define TRUE 0x1
#define FALSE 0x0

#ifdef RISC_V

// The DTEKV-board doesn't have access to the C standard libary.
// Therefore we have to write alternatives in this file
#include "dtekv-lib.h"
void exit(int code);

#else

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sdl-instance.h"
#include "debug.h"

#endif // RISC_V
void sleep_us(size_t microseconds);

#endif // COMMON_H
