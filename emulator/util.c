#include "util.h"
#include "cpu.h"
#include "opcodes.h"
#include "cpu_mem.h"


void log_disassembled_instruction(const CPU *cpu) {
    CPUMemory *mem = cpu->mem;
    uint8_t byte0 = cpu_read_mem_8(mem, cpu->pc);
    uint8_t byte1 = cpu_read_mem_8(mem, cpu->pc + 1);
    uint8_t byte2 = cpu_read_mem_8(mem, cpu->pc + 2);
    Instruction instruction = instruction_lookup[byte0];

    // Print the current PC
    printf("%04X  ", cpu->pc);

    // Print the bytes of the current instruction, for example: 4C F5 C5
    switch (instruction.address_mode) {
    case IMM: case ZP0: case ZPX: case ZPY: case XIN: case YIN: case REL: // Instruction is 2 bytes long
        printf("%02X %02X     ", byte0, byte1);
        break;
    case ABS: case ABX: case ABY: case IND: // Instruction is 3 bytes long
        printf("%02X %02X %02X  ", byte0, byte1, byte2);
        break;
    default: // Instruction is 1 byte long
        printf("%02X        ", byte0);
    }

    // Print the name of the current instruction, for example: JMP
    printf("%s ", opcode_name_lookup[instruction.opcode]);

    switch (instruction.address_mode) {
    case ABS: {
        uint16_t address = (byte2 << 8) | byte1;
        printf("$%04X                       ", address);
        break;
    }
    default: {
        printf("???                         ");
    }
    }


    // Print the state of the CPU before the instruction is executed
    printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%lu\n",
        cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp, cpu->cur_cycle);
}

// Print the address that the instruction operates on
//switch (instruction.address_mode) {
//case IMM:  // Immediate addressing
//    printf("#$%02X", cpu_read_mem_8(cpu->mem, cpu->address));
//    break;
//
//case ZP0:  // Zeropage addressing
//    printf("$%02X", cpu->address & 0xFF);
//    break;
//
//case ZPX:  // Zeropage, X-indexed addressing
//    printf("$%02X,X", cpu->address & 0xFF);
//    break;
//
//case ZPY:  // Zeropage, Y-indexed addressing
//    printf("$%02X,Y", cpu->address & 0xFF);
//    break;
//
//case ABS:  // Absolute addressing
//    uint16_t address =
//    printf("$%04X", cpu->address);
//    break;
//
//case ABX:  // Absolute, X-indexed addressing
//    printf("$%04X,X", cpu->address);
//    break;
//
//case ABY:  // Absolute, Y-indexed addressing
//    printf("$%04X,Y", cpu->address);
//    break;
//
//case IND:  // Indirect addressing
//    printf("($%04X)", cpu->address);
//    break;
//
//case XIN:  // X-indexed, Indirect (Pre-Indexed Indirect)
//    printf("($%02X,X)", cpu->address & 0xFF);
//    break;
//
//case YIN:  // Indirect, Y-indexed (Post-Indexed Indirect)
//    printf("($%02X),Y", cpu->address & 0xFF);
//    break;
//
//case REL:  // Relative addressing
//    printf("$%04X", cpu->address); // absolute address has already been calculated
//    break;
//
//case ACC:  // Accumulator addressing
//    printf("A");
//    break;
//
//case IMP:  // Implied addressing
//    // No operand for implied instructions
//        break;
//
//case UNK:  // Unknown or illegal addressing mode
//default:
//    printf("???");
//    break;
//}