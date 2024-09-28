#ifndef COMMON_H
#define COMMON_H

#ifdef RISC_V

// The DTEKV-board doesn't have access to the C standard libary.
// Therefore we have to write alternatives in this file
#include "dtekv-lib.h"

#else

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdl-gfx.h"

#endif

#endif
