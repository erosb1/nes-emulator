#include "util.h"
#include "cpu.h"
#include "opcodes.h"
#include "mem.h"

#define ADDRESS_MODE_COLUMN_WIDTH 28


int is_illegal(uint8_t byte) {
    Instruction instruction = instruction_lookup[byte];
    return (instruction.opcode >= 56) || // Illegal operand
        (instruction.opcode == NOP && byte != 0xEA) || // Illegal NOP
            byte == 0xEB; // USBC (treated as SBC IMM)
}


/**
 *  This is a rather complex function that logs info about the addressing mode
 *  of the instruction that is currently about to be executed.
 *
 *  This function mimics the behavior of set_address in cpu.c,
 *  without actually updating the internal values of the cpu
 */
static void log_address_mode_info(const CPU *cpu, Instruction instruction) {
    MEM *mem = cpu->mem;
    uint8_t byte1 = mem_read_8(mem, cpu->pc + 1);
    uint8_t byte2 = mem_read_8(mem, cpu->pc + 2);
    size_t cur_column_width = 0;
    uint16_t address = 0x0000;

    switch (instruction.address_mode) {
    case ACC: {
        printf("A ");
        cur_column_width += 2;
        break;
    }
    case ABS: {
        address = (byte2 << 8) | byte1;
        printf("$%04X ", address);
        cur_column_width += 6;
        break;
    }
    case ABX: {
        uint16_t address_pre = (byte2 << 8) | byte1;
        address = address_pre + cpu->x;
        printf("$%04X,X @ %04X ", address_pre, address);
        cur_column_width += 15;
        break;
    }
    case ABY: {
        uint16_t address_pre = (byte2 << 8) | byte1;
        address = address_pre + cpu->y;
        printf("$%04X,Y @ %04X ", address_pre, address);
        cur_column_width += 15;
        break;
    }
    case IMM: {
        address = cpu->pc;
        printf("#$%02X ", byte1);
        cur_column_width += 5;
        break;
    }
    case IMP: {
        break;
    }
    case REL: {
        address = (cpu->pc + 2) + (int8_t) byte1;
        printf("$%04X ", address);
        cur_column_width += 6;
        break;
    }
    case IND: {
        uint16_t address_pre = (byte2 << 8) | byte1;
        address = mem_read_8(mem, address_pre) |
            (mem_read_8(mem, (address_pre & 0xFF00) | ((address_pre + 1) & 0xFF)) << 8);
        printf("($%04X) = %04X ", address_pre, address);
        cur_column_width += 15;
        break;
    }
    case XIN: {
        uint16_t zp_address = (byte1 + cpu->x) & 0xFF;
        uint16_t hi_byte = mem_read_8(mem, (zp_address + 1) & 0xFF);
        uint16_t low_byte = mem_read_8(mem, zp_address & 0xFF);
        address = (hi_byte << 8) | low_byte;
        printf("($%02X,X) @ %02X = %04X ", byte1, zp_address, address);
        cur_column_width += 20;
        break;
    }
    case YIN: {
        uint16_t zp_address = byte1;
        uint16_t hi_byte = mem_read_8(mem, (zp_address + 1) & 0xFF);
        uint16_t low_byte = mem_read_8(mem, zp_address & 0xFF);
        uint16_t real_address = (hi_byte << 8) | low_byte;
        address = (real_address + cpu->y) & 0xFFFF;
        printf("($%02X),Y = %04X @ %04X ", byte1, real_address, address);
        cur_column_width += 22;
        break;
    }
    case ZP0: {
        address = byte1;
        printf("$%02X ", address);
        cur_column_width += 4;
        break;
    }
    case ZPX: {
        address = ((uint16_t)(byte1 + cpu->x)) & 0xFF;
        printf("$%02X,X @ %02X ", byte1, address);
        cur_column_width += 11;
        break;
    }
    case ZPY: {
        address = ((uint16_t)(byte1 + cpu->y)) & 0xFF;
        printf("$%02X,Y @ %02X ", byte1, address);
        cur_column_width += 11;
        break;
    }
    case UNK: default: {
        printf("???");
        cur_column_width += 3;
        break;
    }}

    // This is a rather ugly nested switch statement
    // Some instructions in the log show the value at the address it operates on.
    // This switch statement finds those instructions and prints the value.
    switch (instruction.address_mode) {
    case IMM: case ACC: case IND: case IMP:
        break;
    default:
        switch (instruction.opcode) {
        case STA: case STX: case STY: case BIT: case LDA: case LDX: case LDY: case CPY:
        case AND: case ORA: case EOR: case ADC: case SBC: case CMP: case CPX: case LSR:
        case ASL: case ROR: case ROL: case INC: case DEC: case NOP: case LAX: case SAX:
        case DCP: case ISB: case SLO: case RLA: case SRE: case RRA:
            printf("= %02X", mem_read_8(mem, address));
                cur_column_width += 4;
                break;
        default: break;
        }
    }


    // Print blank spaces so that the entire column width is equal to ADDRESS_MODE_COLUMN_WIDTH
    for (int i = 0; i < ADDRESS_MODE_COLUMN_WIDTH - cur_column_width; i++) {
        printf(" ");
    }
}


void log_disassembled_instruction(const CPU *cpu) {
    MEM *mem = cpu->mem;
    uint8_t byte0 = mem_read_8(mem, cpu->pc);
    uint8_t byte1 = mem_read_8(mem, cpu->pc + 1);
    uint8_t byte2 = mem_read_8(mem, cpu->pc + 2);
    Instruction instruction = instruction_lookup[byte0];

    // Print the current PC
    printf("%04X  ", cpu->pc);

    // Print the bytes of the current instruction, for example: 4C F5 C5
    switch (instruction.address_mode) {
    case IMM: case ZP0: case ZPX: case ZPY: case XIN: case YIN: case REL: // Instruction is 2 bytes long
        printf("%02X %02X    ", byte0, byte1);
        break;
    case ABS: case ABX: case ABY: case IND: // Instruction is 3 bytes long
        printf("%02X %02X %02X ", byte0, byte1, byte2);
        break;
    default: // Instruction is 1 byte long
        printf("%02X       ", byte0);
    }

    // Illegal opcodes are prepended with a '*'
    if (is_illegal(byte0)) printf("*");
    else printf(" ");

    // Print the name of the current instruction, for example: JMP
    printf("%s ", opcode_name_lookup[instruction.opcode]);

    log_address_mode_info(cpu, instruction);

    // Print the state of the CPU before the instruction is executed
    printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%lu\n",
        cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp, cpu->cur_cycle);
}