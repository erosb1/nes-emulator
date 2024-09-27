#include "cpu.h"
#include "opcodes.h"
#include "util.h"

// stack parameters
#define STACK_OFFSET 0x0100

// status masks
#define CARRY_MASK 0x01
#define ZERO_MASK 0x02
#define IRQ_DISABLE_MASK 0x04
#define DECIMAL_MASK 0x08
#define BREAK_MASK 0x10
// bit 5 is ignored
#define OVERFLOW_MASK 0x40
#define NEGATIVE_MASK 0x80

// PPUCTRL
// offset
#define PPUCTRL_OFFSET 0x2000
// masks    TODO: add the rest of these
#define NMI_ENABLE_MASK 0x80

// PPUSTATUS masks
// offset
#define PPUSTATUS_OFFSET 0x2002
// masks    TODO: add the rest of these
#define VBLANK_MASK 0x80

// vector offsets
#define RESET_VECTOR_OFFSET 0xFFFC
#define NMI_VECTOR_OFFSET 0xFFFA

void ppu_vblank_set(uint8_t *cpu_mem, uint8_t bool) {
    if (bool) {
        cpu_mem[PPUCTRL_OFFSET] |= VBLANK_MASK;
    } else {
        cpu_mem[PPUCTRL_OFFSET] &= ~VBLANK_MASK;
    }
}

void ppu_maybe_nmi(CPU *cpu) {
    if (cpu->mem[PPUSTATUS_OFFSET] & NMI_ENABLE_MASK) {
        cpu->pc = load_2_bytes(cpu->mem + NMI_VECTOR_OFFSET);
    }
}

// TODO: add all of these
void cpu_run_instruction(CPU *cpu) {
    uint8_t *mem = cpu->mem;
    uint8_t opcode = mem[cpu->pc];

    switch (opcode) {
    case BRK: { // not tested properly
        mem[STACK_OFFSET + cpu->sp] = cpu->pc + 2;
        mem[STACK_OFFSET + cpu->sp - 1] = cpu->sr | BREAK_MASK;
        cpu->sp -= 3;
        cpu->pc = load_2_bytes(cpu->mem + NMI_VECTOR_OFFSET);
        cpu->sr |= IRQ_DISABLE_MASK;
        cpu->cur_cycle += 7;
        printf("BRK\n");
        break;
    }

    case CLC: {
        cpu->sr &= ~CARRY_MASK;
        cpu->cur_cycle += 2;
        printf("CLC\n");
        break;
    }

    case JSR: {
        mem[STACK_OFFSET + cpu->sp] = cpu->pc + 2;
        cpu->sp -= 2;
        cpu->pc += 1;
        uint16_t jump_addr = load_2_bytes(mem + cpu->pc);
        cpu->pc = jump_addr - 1;
        cpu->cur_cycle += 6;
        printf("JSR $%04hX\n", jump_addr);
        break;
    }

    case SEC: {
        cpu->sr |= CARRY_MASK;
        cpu->cur_cycle += 2;
        printf("SEC\n");
        break;
    }

    case JMP_abs: {
        cpu->pc += 1;
        uint16_t jump_addr = load_2_bytes(mem + cpu->pc);
        cpu->pc = jump_addr - 1;
        cpu->cur_cycle += 3;
        printf("JMP $%04hX\n", jump_addr);
        break;
    }

    case STA_zpg: {
        cpu->pc += 1;
        uint8_t zpg_addr = mem[cpu->pc];
        mem[zpg_addr] = cpu->ac;
        cpu->cur_cycle += 3;
        printf("STA $%02hX\n", zpg_addr);
        break;
    }

    case STX_zpg: {
        cpu->pc += 1;
        uint8_t zpg_addr = mem[cpu->pc];
        mem[zpg_addr] = cpu->x;
        cpu->cur_cycle += 3;
        printf("STX $%02hX\n", zpg_addr);
        break;
    }

    case BCC: {
        cpu->pc += 1;
        int8_t offset = mem[cpu->pc];
        uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
                                                   // instruction + offset
        if (~cpu->sr & CARRY_MASK) {
            cpu->cur_cycle += 3 + ((jump_addr & BYTE_HI_MASK) ==
                                   (cpu->pc & BYTE_LO_MASK)); // 4 if
            // address is on different page
            cpu->pc = jump_addr - 1;
        } else {
            cpu->cur_cycle += 2;
        }
        printf("BCC $%04hX\n", jump_addr);
        break;
    }

    case SEI: {
        cpu->sr |= IRQ_DISABLE_MASK;
        cpu->cur_cycle += 2;
        printf("SEI\n");
        break;
    }

    case BNE: {
        cpu->pc += 1;
        int8_t offset = mem[cpu->pc];
        uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
                                                   // instruction + offset
        if (~cpu->sr & ZERO_MASK) {
            cpu->cur_cycle += 3 + ((jump_addr & BYTE_HI_MASK) ==
                                   (cpu->pc & BYTE_LO_MASK)); // 4 if
            // address is on different page
            cpu->pc = jump_addr - 1;
        } else {
            cpu->cur_cycle += 2;
        }
        printf("BNE $%04hX\n", jump_addr);
        break;
    }

    case CLD: {
        cpu->sr &= ~DECIMAL_MASK;
        cpu->cur_cycle += 2;
        printf("CLD\n");
        break;
    }

    case LDX_imm: {
        cpu->pc += 1;
        int8_t imm = mem[cpu->pc];
        cpu->x = imm;
        if (imm < 0) {
            cpu->sr |= NEGATIVE_MASK;
            cpu->sr &= ~ZERO_MASK;
        } else if (imm == 0) {
            cpu->sr &= ~NEGATIVE_MASK;
            cpu->sr |= ZERO_MASK;
        } else {
            cpu->sr &= ~NEGATIVE_MASK;
            cpu->sr &= ~ZERO_MASK;
        }
        cpu->cur_cycle += 2;
        printf("LDX #$%02hX\n", (uint8_t)imm);
        break;
    }

    case LDA_imm: {
        cpu->pc += 1;
        int8_t imm = mem[cpu->pc];
        cpu->ac = imm;
        if (imm < 0) {
            cpu->sr |= NEGATIVE_MASK;
            cpu->sr &= ~ZERO_MASK;
        } else if (imm == 0) {
            cpu->sr &= ~NEGATIVE_MASK;
            cpu->sr |= ZERO_MASK;
        } else {
            cpu->sr &= ~NEGATIVE_MASK;
            cpu->sr &= ~ZERO_MASK;
        }
        cpu->cur_cycle += 2;
        printf("LDA #$%02hX\n", (uint8_t)imm);
        break;
    }

    case BCS: {
        cpu->pc += 1;
        int8_t offset = mem[cpu->pc];
        uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
                                                   // instruction + offset
        if (cpu->sr & CARRY_MASK) {
            cpu->cur_cycle += 3 + ((jump_addr & BYTE_HI_MASK) ==
                                   (cpu->pc & BYTE_LO_MASK)); // 4 if
            // address is on different page
            cpu->pc = jump_addr - 1;
        } else {
            cpu->cur_cycle += 2;
        }
        printf("BCS $%04hX\n", jump_addr);
        break;
    }

    case NOP: {
        cpu->cur_cycle += 2;
        printf("NOP\n");
        break;
    }

    case BEQ: {
        cpu->pc += 1;
        int8_t offset = mem[cpu->pc];
        uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
                                                   // instruction + offset
        if (cpu->sr & ZERO_MASK) {
            cpu->cur_cycle += 3 + ((jump_addr & BYTE_HI_MASK) ==
                                   (cpu->pc & BYTE_LO_MASK)); // 4 if
            // address is on different page
            cpu->pc = jump_addr - 1;
        } else {
            cpu->cur_cycle += 2;
        }
        printf("BEQ $%04hX\n", jump_addr);
        break;
    }

    default:
        printf("\n");
    }
}

void cpu_run_instructions(CPU *cpu, size_t cycles) {
    while (cpu->cur_cycle < cycles) {

        print_state(cpu);
        cpu_run_instruction(cpu);

        ++cpu->pc;

#ifdef BREAKPOINT
        if (cpu->pc == BREAKPOINT) {
            // set actual breakpoint here for inspecting memory
            exit(EXIT_SUCCESS);
        }
#endif /* ifdef BREAKPOINT */
    }
#ifndef TESTING
    cpu->cur_cycle = 0; // to prevent overflow
#endif                  /* ifndef TESTING */
}
