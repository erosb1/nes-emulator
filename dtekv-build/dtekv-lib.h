#ifndef DTEKV_LIB_H
#define DTEKV_LIB_H

void dtekv_printc(char );
void dtekv_print(char *);
void dtekv_print_dec(unsigned int);
void dtekv_print_hex32 ( unsigned int);

void handle_exception ( unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4, unsigned arg5, unsigned mcause, unsigned syscall_num );
void handle_interrupt(unsigned cause);

#endif

