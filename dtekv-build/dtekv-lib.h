#ifndef DTEKV_LIB_H
#define DTEKV_LIB_H

// Fixed-width integer types
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef uint32_t size_t;

// Limits for these types
#define INT8_MIN (-128)
#define INT8_MAX (127)
#define UINT8_MAX (255U)

#define INT16_MIN (-32768)
#define INT16_MAX (32767)
#define UINT16_MAX (65535U)

#define INT32_MIN (-2147483648)
#define INT32_MAX (2147483647)
#define UINT32_MAX (4294967295U)
#define SIZE_T_MAX (UINT32_MAX)

#define NULL (void *)0

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void exit(int code);

// Does nothing
#define assert(expression) ((void)0)

void dtekv_printc(char);
void dtekv_print(char *);
void dtekv_print_dec(unsigned int);
void dtekv_print_hex32(unsigned int);

// Since we can't access printf from stdio.h
#define printf(fmt, ...) dtekv_print(fmt)

void handle_exception(unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4, unsigned arg5,
                      unsigned mcause, unsigned syscall_num);
void handle_interrupt(unsigned cause);

void *memset(void *dest, int chr, size_t count);
void *memcpy(void *dest, const void *src, size_t count);

#endif
