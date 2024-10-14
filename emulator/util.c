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
    case ACC: {
        printf("A                           ");
        break;
    }
    case ABS: {
        uint16_t address = (byte2 << 8) | byte1;
        switch (instruction.opcode) {
        case STA: case STX: case STY: case BIT: case LDA: case LDX: case LDY: case CPY:
        case AND: case ORA: case EOR: case ADC: case SBC: case CMP: case CPX: case LSR:
        case ASL: case ROR: case ROL: case INC: case DEC:
            printf("$%04X = %02X                  ", address, cpu_read_mem_8(mem, address));
            break;
        default:
            printf("$%04X                       ", address);
        }
        break;
    }
    case ABX: {
        uint16_t address = (byte2 << 8) | byte1;
        uint16_t address_incremented = address + cpu->x;
        switch (instruction.opcode) {
        case STA: case STX: case STY: case BIT: case LDA: case LDX: case LDY: case CPY:
        case AND: case ORA: case EOR: case ADC: case SBC: case CMP: case CPX: case LSR:
        case ASL: case ROR: case ROL: case INC: case DEC:
            printf("$%04X,X @ %04X = %02X         ", address, address_incremented, cpu_read_mem_8(mem, address_incremented));
            break;
        default:
            printf("$%04X                      ", address);
        }
        break;
    }
    case ABY: {
        uint16_t address = (byte2 << 8) | byte1;
        uint16_t address_incremented = address + cpu->y;
        switch (instruction.opcode) {
        case STA: case STX: case STY: case BIT: case LDA: case LDX: case LDY: case CPY:
        case AND: case ORA: case EOR: case ADC: case SBC: case CMP: case CPX: case LSR:
        case ASL: case ROR: case ROL: case INC: case DEC:
            printf("$%04X,Y @ %04X = %02X         ", address, address_incremented, cpu_read_mem_8(mem, address_incremented));
            break;
        default:
            printf("$%04X                      ", address);
        }
        break;
    }
    case IMM: {
        printf("#$%02X                        ", byte1);
        break;
    }
    case IMP: {
        printf("                            ");
        break;
    }
    case REL: {
        uint16_t address = cpu->pc + byte1 + 2;
        printf("$%04X                       ", address);
        break;
    }
    case IND: {
        uint16_t address = (byte2 << 8) | byte1;
        uint16_t indirect_address = cpu_read_mem_8(mem, address) |
            (cpu_read_mem_8(mem, (address & 0xFF00) | ((address + 1) & 0xFF)) << 8);
        printf("($%04X) = %04X              ", address, indirect_address);
        break;
    }
    case XIN: {
        uint16_t zp_address = (byte1 + cpu->x) & 0xFF;
        uint16_t hi_byte = cpu_read_mem_8(mem, (zp_address + 1) & 0xFF);
        uint16_t low_byte = cpu_read_mem_8(mem, zp_address & 0xFF);
        uint16_t real_address = (hi_byte << 8) | low_byte;
        uint8_t value = cpu_read_mem_8(mem, real_address);
        printf("($%02X,X) @ %02X = %04X = %02X    ", byte1, zp_address, real_address, value);
        break;
    }
    case YIN: {
        uint16_t zp_address = byte1;
        uint16_t hi_byte = cpu_read_mem_8(mem, (zp_address + 1) & 0xFF);
        uint16_t low_byte = cpu_read_mem_8(mem, zp_address & 0xFF);
        uint16_t real_address = (hi_byte << 8) | low_byte;
        uint16_t real_address_incremented = (real_address + cpu->y) & 0xFFFF;
        uint8_t value = cpu_read_mem_8(mem, real_address_incremented);
        printf("($%02X),Y = %04X @ %04X = %02X  ", byte1, real_address, real_address_incremented, value);
        break;
    }
    case ZP0: {
        switch (instruction.opcode) {
        case STA: case STX: case STY: case BIT: case LDA: case LDX: case LDY: case CPY:
        case AND: case ORA: case EOR: case ADC: case SBC: case CMP: case CPX: case LSR:
        case ASL: case ROR: case ROL: case INC: case DEC:
            printf("$%02X = %02X                    ", byte1, cpu_read_mem_8(mem, byte1));
            break;
        default:
            printf("$%02X                         ", byte1);
        }
        break;
    }
    case ZPX: {
        uint16_t address = ((uint16_t)(byte1 + cpu->x)) & 0xFF;
        switch (instruction.opcode) {
        case STA: case STX: case STY: case BIT: case LDA: case LDX: case LDY: case CPY:
        case AND: case ORA: case EOR: case ADC: case SBC: case CMP: case CPX: case LSR:
        case ASL: case ROR: case ROL: case INC: case DEC:
            printf("$%02X,X @ %02X = %02X             ", byte1, address, cpu_read_mem_8(mem, address));
            break;
        default:
            printf("$%02X,X @ %02X             ", byte1, address);
        }
        break;
    }
    case ZPY: {
        uint16_t address = ((uint16_t)(byte1 + cpu->y)) & 0xFF;
        switch (instruction.opcode) {
        case STA: case STX: case STY: case BIT: case LDA: case LDX: case LDY: case CPY:
        case AND: case ORA: case EOR: case ADC: case SBC: case CMP: case CPX: case LSR:
        case ASL: case ROR: case ROL: case INC: case DEC:
            printf("$%02X,Y @ %02X = %02X             ", byte1, address, cpu_read_mem_8(mem, address));
            break;
        default:
            printf("$%02X,Y @ %02X             ", byte1, address);
        }
        break;
    }
    default: {
        printf("???                         ");
    }}

    // ($80,X) @ 80 = 0200 = 5A

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