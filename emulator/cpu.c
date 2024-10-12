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
static void set_address(CPU *cpu, AddressMode address_mode) {
    CPUMemory* mem = cpu->mem;

    switch (address_mode) {
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

        // Add an extra cycle if the new address crosses a page boundary
        if ((base_address & 0xFF00) != (cpu->address & 0xFF00)) {
            cpu->cur_cycle++;
        }
        break;
    }
    case ABY: { // Absolute, Y-indexed
        uint16_t base_address = cpu_read_mem_16(mem, cpu->pc);
        cpu->address = base_address + cpu->y;
        cpu->pc += 2;

        // Add an extra cycle if the new address crosses a page boundary
        if ((base_address & 0xFF00) != (cpu->address & 0xFF00)) {
            cpu->cur_cycle++;
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
        cpu->address = cpu_read_mem_16(mem, temp);
        cpu->pc += 2;
        break;
    }
    case XIN: { // X-indexed, Indirect (Pre-Indexed Indirect)
        const uint8_t zp_address = cpu_read_mem_8(mem, cpu->pc) + cpu->x;
        cpu->address = cpu_read_mem_16(mem, zp_address & 0xFF);
        cpu->pc++;
        break;
    }
    case YIN: { // Indirect, Y-indexed (Post-Indexed Indirect)
        const uint8_t zp_address = cpu_read_mem_8(mem, cpu->pc);
        uint16_t base_address = cpu_read_mem_16(mem, zp_address);
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
        cpu->address = (uint16_t) cpu_read_mem_8(mem, cpu->pc) + cpu->x;
        cpu->pc++;
        break;
    }
    case ZPY: { // Zeropage, Y-indexed
        cpu->address = (uint16_t) (cpu_read_mem_8(mem, cpu->pc) + cpu->y);
        cpu->pc++;
        break;
    }
    case UNK: default: // Unkown/Illegal
        printf("Unknown Addressing Mode at PC: 0x%04X, Mode: %d\n", cpu->pc, address_mode);
        exit(EXIT_FAILURE);
    }
}


void cpu_run_instruction(CPU *cpu) {

    if (TESTING) print_state(cpu);

    CPUMemory *mem = cpu->mem;
    uint8_t byte = cpu_read_mem_8(mem, cpu->pc++);
    Instruction instruction = instruction_lookup[byte];
    set_address(cpu, instruction.address_mode);
    cpu->cur_cycle += cycle_lookup[byte];

    if (TESTING) {
        print_disassembled_instruction(cpu, instruction);
        printf("\n");
    }

    switch(instruction.opcode) {
    case ADC: {
        // Todo: Implement ADC
        exit(EXIT_FAILURE);
        break;
    }
    case AND: {
        // Todo: Implement AND
        exit(EXIT_FAILURE);
        break;
    }
    case ASL: {
        // Todo: Implement ASL
        exit(EXIT_FAILURE);
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
        // Todo: Implement BIT
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
        break;
    }
    case BVS: {
        // Todo: Implement BVS
        exit(EXIT_FAILURE);
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
        // Todo: Implement CMP
        exit(EXIT_FAILURE);
        break;
    }
    case CPX: {
        // Todo: Implement CPX
        exit(EXIT_FAILURE);
        break;
    }
    case CPY: {
        // Todo: Implement CPY
        exit(EXIT_FAILURE);
        break;
    }
    case DEC: {
        // Todo: Implement DEC
        exit(EXIT_FAILURE);
        break;
    }
    case DEX: {
        // Todo: Implement DEX
        exit(EXIT_FAILURE);
        break;
    }
    case DEY: {
        // Todo: Implement DEY
        exit(EXIT_FAILURE);
        break;
    }
    case EOR: {
        // Todo: Implement EOR
        exit(EXIT_FAILURE);
        break;
    }
    case INC: {
        // Todo: Implement INC
        exit(EXIT_FAILURE);
        break;
    }
    case INX: {
        // Todo: Implement INX
        exit(EXIT_FAILURE);
        break;
    }
    case INY: {
        // Todo: Implement INY
        exit(EXIT_FAILURE);
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
        // Todo: Implement LSR
        exit(EXIT_FAILURE);
        break;
    }
    case NOP: {
        // Do nothing
        break;
    }
    case ORA: {
        // Todo: Implement ORA
        exit(EXIT_FAILURE);
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
        // Todo: Implement ROL
        exit(EXIT_FAILURE);
        break;
    }
    case ROR: {
        // Todo: Implement ROR
        exit(EXIT_FAILURE);
        break;
    }
    case RTI: {
        // Todo: Implement RTI
        exit(EXIT_FAILURE);
        break;
    }
    case RTS: {
        // Todo: Implement RTS
        exit(EXIT_FAILURE);
        break;
    }
    case SBC: {
        // Todo: Implement SBC
        exit(EXIT_FAILURE);
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
    case ILL: {
        // Todo: Handle illegal opcode
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
