#include "dtekv-lib.h"

#define JTAG_UART ((volatile unsigned int *)0x04000040)
#define JTAG_CTRL ((volatile unsigned int *)0x04000044)

void dtekv_printc(char s) {
    while (((*JTAG_CTRL) & 0xffff0000) == 0)
        ;
    *JTAG_UART = s;
}

void dtekv_print(char *s) {
    while (*s != '\0') {
        dtekv_printc(*s);
        s++;
    }
}

void dtekv_print_dec(unsigned int x) {
    unsigned divident = 1000000000;
    char first = 0;
    do {
        int dv = x / divident;
        if (dv != 0)
            first = 1;
        if (first != 0)
            dtekv_printc(48 + dv);
        x -= dv * divident;
        divident /= 10;
    } while (divident != 0);
    if (first == 0)
        dtekv_printc(48);
}

void dtekv_print_hex32(unsigned int x) {
    dtekv_printc('0');
    dtekv_printc('x');
    for (int i = 7; i >= 0; i--) {
        char hd = (char)((x >> (i * 4)) & 0xf);
        if (hd < 10)
            hd += '0';
        else
            hd += ('A' - 10);
        dtekv_printc(hd);
    }
}

/* function: handle_exception
   Description: This code handles an exception. */
void handle_exception(unsigned arg0, unsigned arg1, unsigned arg2,
                      unsigned arg3, unsigned arg4, unsigned arg5,
                      unsigned mcause, unsigned syscall_num) {
    switch (mcause) {
    case 0:
        dtekv_print("\n[EXCEPTION] Instruction address misalignment. ");
        break;
    case 2:
        dtekv_print("\n[EXCEPTION] Illegal instruction. ");
        break;
    case 11:
        if (syscall_num == 4)
            dtekv_print((char *)arg0);
        if (syscall_num == 11)
            dtekv_printc(arg0);
        return;
        break;
    default:
        dtekv_print("\n[EXCEPTION] Unknown error. ");
        break;
    }

    dtekv_print("Exception Address: ");
    dtekv_print_hex32(arg0);
    dtekv_printc('\n');
    while (1)
        ;
}

void exit(int code) {
    while (1) {
    }
}

void *memset(void *dest, int chr, size_t count) {
    printf("TEST4\n");
    for (size_t i = 0; i < count; ++i) {
        ((unsigned char *)dest)[i] = chr;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    char *d = (char *)dest;
    const char *s = (const char *)src;

    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }

    return dest;
}

void handle_interrupt(unsigned clause) {}
