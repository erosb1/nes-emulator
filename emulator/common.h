#ifndef COMMON_H
#define COMMON_H

#define TRUE 1
#define FALSE 0

// --------------- NES CONSTANTS -------------------------------//
#define NES_SCREEN_WIDTH 256  // pixels
#define NES_SCREEN_HEIGHT 224 // pixels

// Tile
#define TILE_WIDTH 8      // pixels
#define TILE_HEIGHT 8     // pixels
#define TILE_BYTE_SIZE 16 // bytes

// Pattern tables
#define PATTERN_TABLE_WIDTH 16  // tiles
#define PATTERN_TABLE_HEIGHT 16 // tiles

// Nametables
#define NAMETABLE_WIDTH 32           // tiles
#define NAMETABLE_HEIGHT 30          // tiles
#define NAMETABLE_BYTE_SIZE 1024     // bytes
#define ATTRIBUTE_TABLE_BYTE_SIZE 64 // bytes

// NTSC rendering
#define NTSC_FRAME_RATE 60
#define NTSC_FRAME_DURATION 16639 // microseconds

// PAL rendering
#define PAL_FRAM_RATE 50

// --------------- INCLUDES ------------------------------------//
#ifdef RISC_V
// The DTEKV-board doesn't have access to the C standard libary.
// Therefore we have to write alternatives in this file
#include "dtekv-lib.h"
void exit(int code);
#include "nes-controller.h"

#else

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"
#include "sdl-instance.h"

#endif // RISC_V
#endif // COMMON_H
