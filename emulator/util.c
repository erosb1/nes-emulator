#include "util.h"
#include "cpu.h"
#include "cpu_memory.h"

void print_state(CPU *cpu) {
    printf(
        "PC:0x%04hX Opcode:0x%02hX AC:0x%02hX X:0x%02hX Y:0x%02hX SR:0x%02hX "
        "SP:0x%02hX Cycle:%zu \t",
        cpu->pc, cpu_read_mem_8(cpu->mem, cpu->pc), cpu->ac, cpu->x, cpu->y,
        cpu->sr, cpu->sp, cpu->cur_cycle);
}
