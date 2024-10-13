#include "cpu.h"
#include "cpu_mem.h"
#include "opcodes.h"
#include "util.h"


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
#define IRQ_VECTOR_OFFSET 0xFFFE

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

static int get_flag(CPU *cpu, CPUFlag flag) {
    return (cpu->sr & flag) ? 1 : 0;
}

static void set_flag(CPU *cpu, CPUFlag flag, int value) {
    if (value) {
        cpu->sr |= flag;
    } else {
        cpu->sr &= ~flag;
    }
}

// This is a helper function that sets the Z and N flags depending on the `value` integer.
// If value == 0 then Z is set
// If bit 7 in value is set then N is set (indicating a negative number)
static void set_ZN_flags(CPU *cpu, uint8_t value) {
    set_flag(cpu, ZERO_MASK, value == 0);
    set_flag(cpu, NEGATIVE_MASK, value & 0x80);
}

// This function branches if predicate is 1, and doesn't branch if it's 0
// It also correctly updates the cur_cycle counter depending on if we crossed
// page borders or not
static void branch_if(CPU *cpu, int predicate) {
    if (predicate) {
        cpu->cur_cycle++;

        // Add an extra cycle if the branch crosses a page boundary
        if ((cpu->address & 0xFF00) != (cpu->pc & 0xFF00)) {
            cpu->cur_cycle++;
        }

        cpu->pc = cpu->address;
    }
}

// This function sets up the cpu->address variable depending on the addressing mode
// It also updates cpu->cur_cycle if an indirect addressing mode crosses a page boundary
static void set_address(CPU *cpu, Instruction instruction) {
    CPUMemory* mem = cpu->mem;

    switch (instruction.address_mode) {
    case ACC: { // Accumulator
        break;
    }
    case ABS: { // Absolute
        cpu->address = cpu_read_mem_16(mem, cpu->pc);
        cpu->pc += 2;
        break;
    }
    case ABX: { // Absolute, X-indexed
        uint16_t base_address = cpu_read_mem_16(mem, cpu->pc);
        cpu->address = base_address + cpu->x;
        cpu->pc += 2;

        if (instruction.opcode != STA && instruction.opcode != STX && instruction.opcode != STY) {
            if ((base_address & 0xFF00) != (cpu->address & 0xFF00)) {
                cpu->cur_cycle++;
            }
        }
        break;
    }
    case ABY: { // Absolute, Y-indexed
        uint16_t base_address = cpu_read_mem_16(mem, cpu->pc);
        cpu->address = base_address + cpu->y;
        cpu->pc += 2;

        if (instruction.opcode != STA && instruction.opcode != STX && instruction.opcode != STY) {
            if ((base_address & 0xFF00) != (cpu->address & 0xFF00)) {
                cpu->cur_cycle++;
            }
        }
        break;
    }
    case IMM: { // Immediate
        cpu->address = cpu->pc;
        cpu->pc++;
        break;
    }
    case IMP: { // Implied
        break;
    }
    case IND: {  // Indirect
        uint16_t temp = cpu_read_mem_16(mem, cpu->pc);
        uint16_t indirect_address = cpu_read_mem_8(mem, temp) |
                                    (cpu_read_mem_8(mem, (temp & 0xFF00) | ((temp + 1) & 0xFF)) << 8);
        cpu->address = indirect_address;
        cpu->pc += 2;
        break;
    }
    case XIN: { // X-indexed, Indirect (Pre-Indexed Indirect)
        const uint8_t zp_address = (cpu_read_mem_8(mem, cpu->pc) + cpu->x) & 0xFF;
        uint16_t base_address = cpu_read_mem_8(mem, zp_address) |
                                (cpu_read_mem_8(mem, (zp_address + 1) & 0xFF) << 8);
        cpu->address = base_address;
        cpu->pc++;
        break;
    }
    case YIN: { // Indirect, Y-indexed (Post-Indexed Indirect)
        const uint8_t zp_address = cpu_read_mem_8(mem, cpu->pc);
        uint16_t base_address = cpu_read_mem_8(mem, zp_address) |
                                (cpu_read_mem_8(mem, (zp_address + 1) & 0xFF) << 8);
        cpu->address = base_address + cpu->y;
        cpu->pc++;

        // Add an extra cycle if the new address crosses a page boundary
        if ((base_address & 0xFF00) != (cpu->address & 0xFF00)) {
            cpu->cur_cycle++;
        }
        break;
    }
    case REL: { // Relative
        int8_t offset = (int8_t) cpu_read_mem_8(mem, cpu->pc);
        cpu->pc++;
        cpu->address = cpu->pc + offset;
        break;
    }
    case ZP0: { // Zeropage
        cpu->address = (uint16_t) cpu_read_mem_8(mem, cpu->pc);
        cpu->pc++;
        break;
    }
    case ZPX: { // Zeropage, X-indexed
        cpu->address = ((uint16_t)(cpu_read_mem_8(mem, cpu->pc) + cpu->x)) & 0xFF;
        cpu->pc++;
        break;
    }
    case ZPY: { // Zeropage, Y-indexed
        cpu->address = ((uint16_t)(cpu_read_mem_8(mem, cpu->pc) + cpu->y)) & 0xFF;
        cpu->pc++;
        break;
    }
    case UNK: default: // Unkown/Illegal
        printf("Unknown Addressing Mode at PC: 0x%04X, Mode: %d\n", cpu->pc, instruction.address_mode);
        exit(EXIT_FAILURE);
    }
}


void cpu_run_instruction(CPU *cpu) {

    if (TESTING) print_state(cpu);

    CPUMemory *mem = cpu->mem;
    uint8_t byte = cpu_read_mem_8(mem, cpu->pc++);
    Instruction instruction = instruction_lookup[byte];
    set_address(cpu, instruction);
    cpu->cur_cycle += cycle_lookup[byte];

    if (TESTING) {
        print_disassembled_instruction(cpu, instruction);
        printf("\n");
    }

    switch(instruction.opcode) {
    case ADC: {
        uint16_t A = cpu->ac; uint16_t M = cpu_read_mem_8(mem, cpu->address);
        uint16_t R = A + M + get_flag(cpu, CARRY_MASK);
        cpu->ac = R & 0xFF;
        set_flag(cpu, CARRY_MASK, R > 0xFF);
        set_flag(cpu, OVERFLOW_MASK, ~(A ^ M) & (A ^ R) & 0x80);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case AND: {
        cpu->ac &= cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case ASL: {
        uint8_t m = cpu_read_mem_8(mem, cpu->address);
        set_flag(cpu, CARRY_MASK, m & 0x80);
        uint8_t shifted = m << 1;
        set_ZN_flags(cpu, shifted);

        if (instruction.address_mode == ACC)
            cpu->ac = shifted;
        else
            cpu_write_mem_8(mem, cpu->address, shifted);
        break;
    }
    case BCC: {
        // Branch if Carry Clear (C flag = 0)
        branch_if(cpu, !get_flag(cpu, CARRY_MASK));
        break;
    }
    case BCS: {
        // Branch if Carry Set (C flag = 1)
        branch_if(cpu, get_flag(cpu, CARRY_MASK));
        break;
    }
    case BEQ: {
        // Branch if Zero Set (Z flag = 1)
        branch_if(cpu, get_flag(cpu, ZERO_MASK));
        break;
    }
    case BIT: {
        uint8_t op = cpu_read_mem_8(mem, cpu->address);
        set_flag(cpu, ZERO_MASK, (cpu->ac & op) == 0);
        set_flag(cpu, NEGATIVE_MASK, op & 0x80);
        set_flag(cpu, OVERFLOW_MASK, op & 0x40);
        break;
    }
    case BMI: {
        // Branch if Negative Set (N flag = 1)
        branch_if(cpu, get_flag(cpu, NEGATIVE_MASK));
        break;
    }
    case BNE: {
        // Branch if Zero Clear (Z flag = 0)
        branch_if(cpu, !get_flag(cpu, ZERO_MASK));
        break;
    }
    case BPL: {
        // Branch if Negative Clear (N flag = 0)
        branch_if(cpu, !get_flag(cpu, NEGATIVE_MASK));
        break;
    }
    case BRK: {
        cpu->pc++;
        push_stack_16(cpu, cpu->pc);
        push_stack_8(cpu, cpu->sr | BREAK_MASK |  UNUSED_MASK);
        cpu->pc = cpu_read_mem_16(mem, IRQ_VECTOR_OFFSET);
        set_flag(cpu, INTERRUPT_MASK, TRUE);
    }
    case BVC: {
        // Branch if Overflow Clear (V flag = 0)
        branch_if(cpu, !get_flag(cpu, OVERFLOW_MASK));
        break;
    }
    case BVS: {
        // Branch if Overflow Set (V flag = 1)
        branch_if(cpu, get_flag(cpu, OVERFLOW_MASK));
        break;
    }
    case CLC: {
        set_flag(cpu, CARRY_MASK, FALSE);
        break;
    }
    case CLD: {
        set_flag(cpu, DECIMAL_MASK, FALSE);
        break;
    }
    case CLI: {
        set_flag(cpu, INTERRUPT_MASK, FALSE);
        break;
    }
    case CLV: {
        set_flag(cpu, OVERFLOW_MASK, FALSE);
        break;
    }
    case CMP: {
        uint16_t a = cpu->ac;
        uint16_t m = cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, (a - m) & 0xFF);
        set_flag(cpu, CARRY_MASK, a >= m);
        break;
    }
    case CPX: {
        uint16_t x = cpu->x;
        uint16_t m = cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, (x - m) & 0xFF);
        set_flag(cpu, CARRY_MASK, x >= m);
        break;
    }
    case CPY: {
        uint16_t y = cpu->y;
        uint16_t m = cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, (y - m) & 0xFF);
        set_flag(cpu, CARRY_MASK, y >= m);
        break;
    }
    case DEC: {
        uint8_t decremented = cpu_read_mem_8(mem, cpu->address) - 1;
        cpu_write_mem_8(mem, cpu->address, decremented);
        set_ZN_flags(cpu, decremented);
        break;
    }
    case DEX: {
        cpu->x--;
        set_ZN_flags(cpu, cpu->x);
        break;
    }
    case DEY: {
        cpu->y--;
        set_ZN_flags(cpu, cpu->y);
        break;
    }
    case EOR: {
        cpu->ac ^= cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case INC: {
        uint8_t incremented = cpu_read_mem_8(mem, cpu->address) + 1;
        cpu_write_mem_8(mem, cpu->address, incremented);
        set_ZN_flags(cpu, incremented);
        break;
    }
    case INX: {
        cpu->x++;
        set_ZN_flags(cpu, cpu->x);
        break;
    }
    case INY: {
        cpu->y++;
        set_ZN_flags(cpu, cpu->y);
        break;
    }
    case JMP: {
        cpu->pc = cpu->address;
        break;
    }
    case JSR: {
        // The return address should be (pc - 1) since the CPU will resume execution in the address immediately after the return address
        push_stack_16(cpu, cpu->pc - 1);
        cpu->pc = cpu->address;
        break;
    }
    case LDA: {
        cpu->ac = cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case LDX: {
        cpu->x = cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->x);
        break;
    }
    case LDY: {
        cpu->y = cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->y);
        break;
    }
    case LSR: {
        uint8_t m = cpu_read_mem_8(mem, cpu->address);
        set_flag(cpu, CARRY_MASK, m & 0x1);
        uint8_t shifted = m >> 1;
        set_ZN_flags(cpu, shifted);

        if (instruction.address_mode == ACC) //
            cpu->ac = shifted;
        else
            cpu_write_mem_8(mem, cpu->address, shifted);
        break;
    }
    case NOP: {
        // Do nothing
        break;
    }
    case ORA: {
        cpu->ac |= cpu_read_mem_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case PHA: {
        push_stack_8(cpu, cpu->ac);
        break;
    }
    case PHP: {
        // BREAK and UNUSED should be set to 1 when pushed
        // src: https://www.masswerk.at/6502/6502_instruction_set.html#PHP
        push_stack_8(cpu, cpu->sr | BREAK_MASK |  UNUSED_MASK);
        break;
    }
    case PLA: {
        cpu->ac = pop_stack_8(cpu);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case PLP: {
        // Pop all flags from the stack
        // Except for BREAK and UNUSED as these should not be modified by the pop operation
        // src: https://www.masswerk.at/6502/6502_instruction_set.html#PLP
        cpu->sr = (cpu->sr & (BREAK_MASK | UNUSED_MASK)) |
                (pop_stack_8(cpu) & ~(BREAK_MASK | UNUSED_MASK));
        break;
    }
    case ROL: {
        uint8_t m = cpu_read_mem_8(mem, cpu->address);
        uint8_t rotated = (m << 1) | get_flag(cpu, CARRY_MASK);
        set_flag(cpu, CARRY_MASK, m & 0x80);
        set_ZN_flags(cpu, rotated);

        if (instruction.address_mode == ACC)
            cpu->ac = rotated;
        else
            cpu_write_mem_8(mem, cpu->address, rotated);
        break;
    }
    case ROR: {
        uint8_t m = cpu_read_mem_8(mem, cpu->address);
        uint8_t rotated = (get_flag(cpu, CARRY_MASK) << 7) | (m >> 1);
        set_flag(cpu, CARRY_MASK, m & 0x1);
        set_ZN_flags(cpu, rotated);

        if (instruction.address_mode == ACC)
            cpu->ac = rotated;
        else
            cpu_write_mem_8(mem, cpu->address, rotated);
        break;
    }
    case RTI: {
        cpu->sr = (cpu->sr & (BREAK_MASK | UNUSED_MASK)) |
               (pop_stack_8(cpu) & ~(BREAK_MASK | UNUSED_MASK));
        cpu->pc = pop_stack_16(cpu);
        break;
    }
    case RTS: {
        // return the stored address and continue execution from the address after it
        cpu->pc = pop_stack_16(cpu) + 1;
        break;
    }
    case SBC: {
        // Subtraction is addition of the two's complement of M
        uint16_t A = cpu->ac;
        uint16_t M = cpu_read_mem_8(mem, cpu->address);
        uint16_t R = A + (M ^ 0xFF) + get_flag(cpu, CARRY_MASK);
        cpu->ac = R & 0xFF;
        set_flag(cpu, CARRY_MASK, R > 0xFF);
        set_flag(cpu, OVERFLOW_MASK, ((A ^ R) & (A ^ M) & 0x80) != 0);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case SEC: {
        set_flag(cpu, CARRY_MASK, TRUE);
        break;
    }
    case SED: {
        set_flag(cpu, DECIMAL_MASK, TRUE);
        break;
    }
    case SEI: {
        set_flag(cpu, INTERRUPT_MASK, TRUE);
        break;
    }
    case STA: {
        cpu_write_mem_8(mem, cpu->address, cpu->ac);
        break;
    }
    case STX: {
        cpu_write_mem_8(mem, cpu->address, cpu->x);
        break;
    }
    case STY: {
        cpu_write_mem_8(mem, cpu->address, cpu->y);
        break;
    }
    case TAX: {
        cpu->x = cpu->ac;
        set_ZN_flags(cpu, cpu->x);
        break;
    }
    case TAY: {
        cpu->y = cpu->ac;
        set_ZN_flags(cpu, cpu->y);
        break;
    }
    case TSX: {
        cpu->x = cpu->sp;
        set_ZN_flags(cpu, cpu->x);
        break;
    }
    case TXA: {
        cpu->ac = cpu->x;
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case TXS: {
        cpu->sp = cpu->x;
        break;
    }
    case TYA: {
        cpu->ac = cpu->y;
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    default: {
        printf("Illegal Instruction at PC: $%04X, Opcode: %02X (Treated as NOP)\n", cpu->pc, byte);
        exit(EXIT_FAILURE);
        break;
    }}
}


void cpu_run_instructions(CPU *cpu, size_t cycles) {
    while (cpu->cur_cycle < cycles) {
        cpu_run_instruction(cpu);

#ifdef BREAKPOINT
        if (cpu->pc == BREAKPOINT) {
            exit(EXIT_SUCCESS);
        }
#endif /* ifdef BREAKPOINT */
    }
}
