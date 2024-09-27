#ifndef CONSOLE_H
#define CONSOLE_H

#include "common.h"

#ifdef RISC_V

#define print(fmt, ...) dtekv_print(fmt)
#else
#define print(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

#endif // CONSOLE_H
