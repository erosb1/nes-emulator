#include "util.h"
#include "cpu.h"
#include "opcodes.h"
#include "cpu_mem.h"

void print_state(CPU *cpu) {
    printf(
        "PC:0x%04hX Opcode:0x%02hX AC:0x%02hX X:0x%02hX Y:0x%02hX SR:0x%02hX "
        "SP:0x%02hX Cycle:%zu \t",
        cpu->pc, cpu_read_mem_8(cpu->mem, cpu->pc), cpu->ac, cpu->x, cpu->y,
        cpu->sr, cpu->sp, cpu->cur_cycle);
}

void print_disassembled_instruction(CPU *cpu, const Instruction instruction) {
    printf("%s ", opcode_name_lookup[instruction.opcode]);

    switch (instruction.address_mode) {
        case IMM:  // Immediate addressing
            printf("#$%02X", cpu_read_mem_8(cpu->mem, cpu->address));
            break;

        case ZP0:  // Zeropage addressing
            printf("$%02X", cpu->address & 0xFF);
            break;

        case ZPX:  // Zeropage, X-indexed addressing
            printf("$%02X,X", cpu->address & 0xFF);
            break;

        case ZPY:  // Zeropage, Y-indexed addressing
            printf("$%02X,Y", cpu->address & 0xFF);
            break;

        case ABS:  // Absolute addressing
            printf("$%04X", cpu->address);
            break;

        case ABX:  // Absolute, X-indexed addressing
            printf("$%04X,X", cpu->address);
            break;

        case ABY:  // Absolute, Y-indexed addressing
            printf("$%04X,Y", cpu->address);
            break;

        case IND:  // Indirect addressing
            printf("($%04X)", cpu->address);
            break;

        case XIN:  // X-indexed, Indirect (Pre-Indexed Indirect)
            printf("($%02X,X)", cpu->address & 0xFF);
            break;

        case YIN:  // Indirect, Y-indexed (Post-Indexed Indirect)
            printf("($%02X),Y", cpu->address & 0xFF);
            break;

        case REL:  // Relative addressing
            printf("$%04X", cpu->address); // absolute address has already been calculated
            break;

        case ACC:  // Accumulator addressing
            printf("A");
            break;

        case IMP:  // Implied addressing
            // No operand for implied instructions
            break;

        case UNK:  // Unknown or illegal addressing mode
        default:
            printf("???");
            break;
    }

    // Handle special cases like STA/STX/STY or other instructions that store to memory
    if (instruction.opcode == STA || instruction.opcode == STX || instruction.opcode == STY) {
        printf(" = %02X", cpu_read_mem_8(cpu->mem, cpu->address));
    }
}

