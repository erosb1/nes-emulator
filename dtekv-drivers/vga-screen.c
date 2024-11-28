
#include "vga-screen.h"
#include "common.h"

#define VGA_SCREEN_WIDTH 320
#define VGA_SCREEN_HEIGHT 240
#define VGA_PIXEL_BUFFER_HEIGHT 480

volatile uint8_t *VGA = (volatile uint8_t *)0x08000000;
volatile uint32_t *VGA_CTRL = (volatile uint32_t *)0x04000100;

void vga_screen_clear() {
    for (uint32_t i = 0; i < VGA_SCREEN_WIDTH * VGA_SCREEN_HEIGHT; i++)
        VGA[i] = 0x00;
}

void vga_screen_put_pixel(uint32_t x, uint32_t y, uint8_t c) {
    if (x >= VGA_SCREEN_WIDTH || y >= VGA_SCREEN_HEIGHT)
        return;
    uint32_t i = y * VGA_SCREEN_WIDTH + x;
    VGA[i] = c;
}