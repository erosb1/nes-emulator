#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef RISC_V

#include "dtekv-lib.h"
#define print(fmt, ...) dtekv_print(fmt)

#else

#include <stdio.h>
#define print(fmt, ...) printf(fmt, ##__VA_ARGS__)

#endif

#endif // CONSOLE_H
