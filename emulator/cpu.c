#include "cpu.h"
#include "cpu_mem.h"
#include "opcodes.h"
#include "util.h"

// options
#define BREAKPOINT 0xC783 // uncomment to run normally

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

void ppu_vblank_set(CPUMemory *mem, uint8_t bool) {
    uint8_t ppuctrl = cpu_read_mem_8(mem, PPUCTRL_OFFSET);
    if (bool) {
        cpu_write_mem_8(mem, PPUCTRL_OFFSET, ppuctrl | VBLANK_MASK);
    } else {
        cpu_write_mem_8(mem, PPUCTRL_OFFSET, ppuctrl & ~VBLANK_MASK);
    }
}

void ppu_maybe_nmi(CPU *cpu) {
    if (cpu_read_mem_8(cpu->mem, PPUSTATUS_OFFSET) & NMI_ENABLE_MASK) {
        cpu->pc = cpu_read_mem_16(cpu->mem, NMI_VECTOR_OFFSET);
    }
}

static int get_flag(CPU *cpu, CPUFlag flag) { return cpu->sr & flag; }

static void set_flag(CPU *cpu, CPUFlag flag, int value) {
    if (value) {
        cpu->sr |= flag;
    } else {
        cpu->sr &= ~flag;
    }
}

// TODO: add all of these
void cpu_run_instruction(CPU *cpu) {
    CPUMemory *mem = cpu->mem;
    uint8_t opcode = cpu_read_mem_8(mem, cpu->pc);

    switch(opcode) {
    case ADC: {
        // Todo: Implement ADC
        break;
    }
    case AND: {
        // Todo: Implement AND
        break;
    }
    case ASL: {
        // Todo: Implement ASL
        break;
    }
    case BCC: {
        cpu->pc += 1;
        uint8_t offset = cpu_read_mem_8(mem, cpu->pc);
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
    case BCS: {
        cpu->pc += 1;
        uint8_t offset = cpu_read_mem_8(mem, cpu->pc);
        uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
        // instruction + offset
        if (get_flag(cpu, CARRY_MASK)) {
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
    case BEQ: {
        cpu->pc += 1;
        uint8_t offset = cpu_read_mem_8(mem, cpu->pc);
        uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
        // instruction + offset
        if (get_flag(cpu, ZERO_MASK)) {
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
    case BIT: {
        // Todo: Implement BIT
        break;
    }
    case BMI: {
        // Todo: Implement BMI
        break;
    }
    case BNE: {
        cpu->pc += 1;
        uint8_t offset = cpu_read_mem_8(mem, cpu->pc);
        uint16_t jump_addr = cpu->pc + 1 + offset; // pc pointing to next
        // instruction + offset
        if (!get_flag(cpu, ZERO_MASK)) {
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
    case BPL: {
        // Todo: Implement BPL
        break;
    }
    case BRK: {
        push_stack_16(cpu, cpu->pc + 2);
        push_stack_8(cpu, cpu->sr | BREAK_MASK);
        cpu->pc = cpu_read_mem_16(mem, NMI_VECTOR_OFFSET);
        cpu->sr |= INTERRUPT_MASK;
        cpu->cur_cycle += 7;
        printf("BRK\n");
        break;
    }
    case BVC: {
        // Todo: Implement BVC
        break;
    }
    case BVS: {
        // Todo: Implement BVS
        break;
    }
    case CLC: {
        set_flag(cpu, CARRY_MASK, FALSE);
        cpu->cur_cycle += 2;
        printf("CLC\n");
        break;
    }
    case CLD: {
        set_flag(cpu, DECIMAL_MASK, FALSE);
        cpu->cur_cycle += 2;
        printf("CLD\n");
        break;
    }
    case CLI: {
        // Todo: Implement CLI
        break;
    }
    case CLV: {
        // Todo: Implement CLV
        break;
    }
    case CMP: {
        // Todo: Implement CMP
        break;
    }
    case CPX: {
        // Todo: Implement CPX
        break;
    }
    case CPY: {
        // Todo: Implement CPY
        break;
    }
    case DEC: {
        // Todo: Implement DEC
        break;
    }
    case DEX: {
        // Todo: Implement DEX
        break;
    }
    case DEY: {
        // Todo: Implement DEY
        break;
    }
    case EOR: {
        // Todo: Implement EOR
        break;
    }
    case INC: {
        // Todo: Implement INC
        break;
    }
    case INX: {
        // Todo: Implement INX
        break;
    }
    case INY: {
        // Todo: Implement INY
        break;
    }
    case JMP: {
        // ABS
        cpu->pc += 1;
        uint16_t jump_addr = cpu_read_mem_16(mem, cpu->pc);
        cpu->pc = jump_addr - 1;
        cpu->cur_cycle += 3;
        printf("JMP $%04hX\n", jump_addr);
        break;
    }
    case JSR: {
        push_stack_16(cpu, cpu->pc + 2);
        cpu->pc += 1;
        uint16_t jump_addr = cpu_read_mem_16(mem, cpu->pc);
        cpu->pc = jump_addr - 1;
        cpu->cur_cycle += 6;
        printf("JSR $%04hX\n", jump_addr);
        break;
    }
    case LDA: {

        // IMM
        cpu->pc += 1;
        uint8_t imm = cpu_read_mem_8(mem, cpu->pc);
        cpu->ac = imm;
        if (imm < 0) {
            set_flag(cpu, NEGATIVE_MASK, TRUE);
            set_flag(cpu, ZERO_MASK, FALSE);
        } else if (imm == 0) {
            set_flag(cpu, NEGATIVE_MASK, FALSE);
            set_flag(cpu, ZERO_MASK, TRUE);
        } else {
            set_flag(cpu, NEGATIVE_MASK, FALSE);
            set_flag(cpu, ZERO_MASK, FALSE);
        }
        cpu->cur_cycle += 2;
        printf("LDA #$%02hX\n", (uint8_t)imm);
        ;
        break;
    }
    case LDX: {
        // IMM
        cpu->pc += 1;
        uint8_t imm = cpu_read_mem_8(mem, cpu->pc);
        cpu->x = imm;
        if (imm < 0) {
            set_flag(cpu, NEGATIVE_MASK, TRUE);
            set_flag(cpu, ZERO_MASK, FALSE);
        } else if (imm == 0) {
            set_flag(cpu, NEGATIVE_MASK, FALSE);
            set_flag(cpu, ZERO_MASK, TRUE);
        } else {
            set_flag(cpu, NEGATIVE_MASK, FALSE);
            set_flag(cpu, ZERO_MASK, FALSE);
        }
        cpu->cur_cycle += 2;
        printf("LDX #$%02hX\n", (uint8_t)imm);
        break;
    }
    case LDY: {
        // Todo: Implement LDY
        break;
    }
    case LSR: {
        // Todo: Implement LSR
        break;
    }
    case NOP: {
        cpu->cur_cycle += 2;
        printf("NOP\n");
        break;
    }
    case ORA: {
        // Todo: Implement ORA
        break;
    }
    case PHA: {
        // Todo: Implement PHA
        break;
    }
    case PHP: {
        // Todo: Implement PHP
        break;
    }
    case PLA: {
        // Todo: Implement PLA
        break;
    }
    case PLP: {
        // Todo: Implement PLP
        break;
    }
    case ROL: {
        // Todo: Implement ROL
        break;
    }
    case ROR: {
        // Todo: Implement ROR
        break;
    }
    case RTI: {
        // Todo: Implement RTI
        break;
    }
    case RTS: {
        // Todo: Implement RTS
        break;
    }
    case SBC: {
        // Todo: Implement SBC
        break;
    }
    case SEC: {
        set_flag(cpu, CARRY_MASK, TRUE);
        cpu->cur_cycle += 2;
        printf("SEC\n");
        break;
    }
    case SED: {
        // Todo: Implement SED
        break;
    }
    case SEI: {
        set_flag(cpu, INTERRUPT_MASK, TRUE);
        cpu->cur_cycle += 2;
        printf("SEI\n");
        break;
    }
    case STA: {
        cpu->pc += 1;
        uint8_t zpg_addr = cpu_read_mem_8(mem, cpu->pc);
        cpu_write_mem_8(mem, zpg_addr, cpu->ac);
        cpu->cur_cycle += 3;
        printf("STA $%02hX\n", zpg_addr);
        break;
    }
    case STX: {
        // zpg
        cpu->pc += 1;
        uint8_t zpg_addr = cpu_read_mem_8(mem, cpu->pc);
        cpu_write_mem_8(mem, zpg_addr, cpu->x);
        cpu->cur_cycle += 3;
        printf("STX $%02hX\n", zpg_addr);
        break;
    }
    case STY: {
        // Todo: Implement STY
        break;
    }
    case TAX: {
        // Todo: Implement TAX
        break;
    }
    case TAY: {
        // Todo: Implement TAY
        break;
    }
    case TSX: {
        // Todo: Implement TSX
        break;
    }
    case TXA: {
        // Todo: Implement TXA
        break;
    }
    case TXS: {
        // Todo: Implement TXS
        break;
    }
    case TYA: {
        // Todo: Implement TYA
        break;
    }
    case ILL: {
        // Todo: Handle illegal opcode
        break;
    }}
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
