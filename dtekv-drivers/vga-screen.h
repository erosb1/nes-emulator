#ifndef VGA_SCREEN_H
#define VGA_SCREEN_H

#include "common.h"

// Drivers för VGA skärmen (denna fil körs endast på DTEKV brädan)

// VGA SCREEN RESOLUTION 320x240

void vga_screen_clear();

void vga_screen_put_pixel(uint32_t x, uint32_t y, uint8_t c);

#endif