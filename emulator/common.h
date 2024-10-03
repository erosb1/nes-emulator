#ifndef COMMON_H
#define COMMON_H

#ifdef RISC_V

// The DTEKV-board doesn't have access to the C standard libary.
// Therefore we have to write alternatives in this file
#include "dtekv-lib.h"
void delay(int64_t microseconds); // TODO: implement

#else

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdl-instance.h"
void delay(int64_t microseconds); // TODO: implement

#endif

#endif
