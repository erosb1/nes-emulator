#include "cpu.h"
#include "mem.h"
#include "opcodes.h"

#include "emulator.h"

// CPU init values
#define SR_INIT_VALUE 0x24
#define SP_INIT_VALUE 0xFD

// vector offsets
#define RESET_VECTOR_OFFSET 0xFFFC
#define NMI_VECTOR_OFFSET 0xFFFA
#define IRQ_VECTOR_OFFSET 0xFFFE

// --------------- STATIC FORWARD DECLARATIONS ---------------- //
static int get_flag(CPU *cpu, CPUFlag flag);
static void set_flag(CPU *cpu, CPUFlag flag, int value);
static void set_ZN_flags(CPU *cpu, uint8_t value);
static int crosses_page_borders(uint16_t address_1, uint16_t address_2);
static int crosses_page_borders_(uint16_t address_1, uint16_t address_2,
                                 Opcode opcode); // only returns true for specific addresses
static void branch_if(CPU *cpu, int predicate);
static uint8_t shift_left(CPU *cpu, uint8_t val);
static uint8_t shift_right(CPU *cpu, uint8_t val);
static uint8_t rotate_left(CPU *cpu, uint8_t val);
static uint8_t rotate_right(CPU *cpu, uint8_t val);
static void execute_instruction(CPU *cpu, Instruction instruction);
static void set_address(CPU *cpu, Instruction instruction);
static void handle_interrupt(CPU *cpu);

// --------------- PUBLIC FUNCTIONS --------------------------- //
void cpu_init(Emulator *emulator) {
    CPU *cpu = &emulator->cpu;
    cpu->emulator = emulator;

    cpu->ac = cpu->x = cpu->y = 0x00;
    cpu->total_cycles = 0;
    cpu->cycles = 0;
    cpu->sr = SR_INIT_VALUE;
    cpu->sp = SP_INIT_VALUE;
    cpu->pending_interrupt = NONE;
    cpu->pc = mem_read_16(&emulator->mem, RESET_VECTOR_OFFSET);

    cpu->is_logging = 0;
}

void cpu_run_cycle(CPU *cpu) {
    MEM *mem = &cpu->emulator->mem;

    if (cpu->cycles == 0) {
        if (cpu->pending_interrupt != NONE) {
            handle_interrupt(cpu); // might add 7 cycles
        }

#ifndef RISC_V
        if (cpu->is_logging)
            debug_log_instruction(cpu);
#endif // RISC_V

        uint8_t byte = mem_read_8(mem, cpu->pc++);
        cpu->cycles += cycle_lookup[byte];
        set_flag(cpu, UNUSED, TRUE);
        Instruction instruction = instruction_lookup[byte];
        set_address(cpu, instruction);         // might add 1 cycle
        execute_instruction(cpu, instruction); // might add 1 cycle

        cpu->total_cycles += cpu->cycles;
    }

    cpu->cycles--;
}

void cpu_set_interrupt(CPU *cpu, Interrupt interrupt) { cpu->pending_interrupt = interrupt; }

// --------------- STATIC FUNCTIONS --------------------------- //
static int get_flag(CPU *cpu, CPUFlag flag) { return (cpu->sr & flag) ? 1 : 0; }

static void set_flag(CPU *cpu, CPUFlag flag, int value) {
    if (value) {
        cpu->sr |= flag;
    } else {
        cpu->sr &= ~flag;
    }
}

// This is a helper function that sets the Z and N flags depending on the
// `value` integer. If value == 0 then Z is set If bit 7 in value is set then N
// is set (indicating a negative number)
static void set_ZN_flags(CPU *cpu, uint8_t value) {
    set_flag(cpu, ZERO, value == 0);
    set_flag(cpu, NEGATIVE, value & 0x80);
}

int crosses_page_borders(uint16_t address_1, uint16_t address_2) {
    return (address_1 & 0xFF00) != (address_2 & 0xFF00);
}

// NES quirk, some opcodes are excluded
int crosses_page_borders_(uint16_t address_1, uint16_t address_2, Opcode opcode) {
    // clang-format off
    switch (opcode) {
    case STA: case ASL: case DEC: case INC: case LSR: case ROL: case ROR:
    case SLO: case RLA: case SRE: case RRA: case DCP: case ISB: case SHY:
        return 0;
    default: break;
    }
    // clang-format on
    return (address_1 & 0xFF00) != (address_2 & 0xFF00);
}

uint8_t shift_left(CPU *cpu, uint8_t val) {
    set_flag(cpu, CARRY, val & 0x80);
    val <<= 1;
    set_ZN_flags(cpu, val);
    return val;
}

uint8_t shift_right(CPU *cpu, uint8_t val) {
    set_flag(cpu, CARRY, val & 0x01);
    val >>= 1;
    set_ZN_flags(cpu, val);
    return val;
}

uint8_t rotate_left(CPU *cpu, uint8_t val) {
    uint8_t rotated = val << 1;
    rotated |= cpu->sr & CARRY;
    set_flag(cpu, CARRY, val & 0x80);
    set_ZN_flags(cpu, rotated);
    return rotated;
}

uint8_t rotate_right(CPU *cpu, uint8_t val) {
    uint8_t rotated = val >> 1;
    rotated |= (cpu->sr & CARRY) << 7;
    set_flag(cpu, CARRY, val & 0x01);
    set_ZN_flags(cpu, rotated);
    return rotated;
}

// This function branches if predicate is 1, and doesn't branch if it's 0
// It also correctly updates the cur_cycle counter depending on if we crossed
// page borders or not
static void branch_if(CPU *cpu, int predicate) {
    if (predicate) {
        cpu->cycles++;

        // Add an extra cycle if the branch crosses a page boundary
        if (crosses_page_borders(cpu->pc, cpu->address)) {
            cpu->cycles++;
        }

        cpu->pc = cpu->address;
    }
}

void execute_instruction(CPU *cpu, Instruction instruction) {
    MEM *mem = &cpu->emulator->mem;

    switch (instruction.opcode) {
    case ADC: {
        uint16_t A = cpu->ac;
        uint16_t M = mem_read_8(mem, cpu->address);
        uint16_t R = A + M + get_flag(cpu, CARRY);
        cpu->ac = R & 0xFF;
        set_flag(cpu, CARRY, R > 0xFF);
        set_flag(cpu, OVERFLW, ((~(A ^ M) & (A ^ R) & 0x80) != 0));
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case AND: {
        cpu->ac &= mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case ASL: {
        if (instruction.address_mode == ACC) {
            cpu->ac = shift_left(cpu, cpu->ac);
        } else {
            uint8_t m = mem_read_8(mem, cpu->address);
            mem_write_8(mem, cpu->address, m); // Dummy
            mem_write_8(mem, cpu->address, shift_left(cpu, m));
        }
        break;
    }
    case BCC: {
        // Branch if Carry Clear (C flag = 0)
        branch_if(cpu, !get_flag(cpu, CARRY));
        break;
    }
    case BCS: {
        // Branch if Carry Set (C flag = 1)
        branch_if(cpu, get_flag(cpu, CARRY));
        break;
    }
    case BEQ: {
        // Branch if Zero Set (Z flag = 1)
        branch_if(cpu, get_flag(cpu, ZERO));
        break;
    }
    case BIT: {
        uint8_t op = mem_read_8(mem, cpu->address);
        set_flag(cpu, ZERO, (cpu->ac & op) == 0);
        set_flag(cpu, NEGATIVE, op & 0x80);
        set_flag(cpu, OVERFLW, op & 0x40);
        break;
    }
    case BMI: {
        // Branch if Negative Set (N flag = 1)
        branch_if(cpu, get_flag(cpu, NEGATIVE));
        break;
    }
    case BNE: {
        // Branch if Zero Clear (Z flag = 0)
        branch_if(cpu, !get_flag(cpu, ZERO));
        break;
    }
    case BPL: {
        // Branch if Negative Clear (N flag = 0)
        branch_if(cpu, !get_flag(cpu, NEGATIVE));
        break;
    }
    case BRK: {
        cpu->pc++;
        mem_push_stack_16(cpu, cpu->pc);
        mem_push_stack_8(cpu, cpu->sr | BREAK | UNUSED);
        cpu->pc = mem_read_16(mem, IRQ_VECTOR_OFFSET);
        set_flag(cpu, INTERRUPT, TRUE);
        break;
    }
    case BVC: {
        // Branch if Overflow Clear (V flag = 0)
        branch_if(cpu, !get_flag(cpu, OVERFLW));
        break;
    }
    case BVS: {
        // Branch if Overflow Set (V flag = 1)
        branch_if(cpu, get_flag(cpu, OVERFLW));
        break;
    }
    case CLC: {
        set_flag(cpu, CARRY, FALSE);
        break;
    }
    case CLD: {
        set_flag(cpu, DECIMAL, FALSE);
        break;
    }
    case CLI: {
        set_flag(cpu, INTERRUPT, FALSE);
        break;
    }
    case CLV: {
        set_flag(cpu, OVERFLW, FALSE);
        break;
    }
    case CMP: {
        uint16_t a = cpu->ac;
        uint16_t m = mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, (a - m) & 0xFF);
        set_flag(cpu, CARRY, a >= m);
        break;
    }
    case CPX: {
        uint16_t x = cpu->x;
        uint16_t m = mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, (x - m) & 0xFF);
        set_flag(cpu, CARRY, x >= m);
        break;
    }
    case CPY: {
        uint16_t y = cpu->y;
        uint16_t m = mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, (y - m) & 0xFF);
        set_flag(cpu, CARRY, y >= m);
        break;
    }
    case DEC: {
        uint8_t decremented = mem_read_8(mem, cpu->address) - 1;
        mem_write_8(mem, cpu->address, decremented);
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
        cpu->ac ^= mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case INC: {
        uint8_t incremented = mem_read_8(mem, cpu->address) + 1;
        mem_write_8(mem, cpu->address, incremented);
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
        // The return address should be (pc - 1) since the CPU will resume
        // execution in the address immediately after the return address
        mem_push_stack_16(cpu, cpu->pc - 1);
        cpu->pc = cpu->address;
        break;
    }
    case LDA: {
        cpu->ac = mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case LDX: {
        cpu->x = mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->x);
        break;
    }
    case LDY: {
        cpu->y = mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->y);
        break;
    }
    case LSR: {
        if (instruction.address_mode == ACC)
            cpu->ac = shift_right(cpu, cpu->ac);
        else {
            uint8_t m = mem_read_8(mem, cpu->address);
            mem_write_8(mem, cpu->address, shift_right(cpu, m));
        }
        break;
    }
    case NOP: {
        // Do nothing
        break;
    }
    case ORA: {
        cpu->ac |= mem_read_8(mem, cpu->address);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case PHA: {
        mem_push_stack_8(cpu, cpu->ac);
        break;
    }
    case PHP: {
        // BREAK and UNUSED should be set to 1 when pushed
        // src: https://www.masswerk.at/6502/6502_instruction_set.html#PHP
        mem_push_stack_8(cpu, cpu->sr | BREAK | UNUSED);
        break;
    }
    case PLA: {
        cpu->ac = mem_pop_stack_8(cpu);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case PLP: {
        // Pop all flags from the stack
        // Except for BREAK and UNUSED as these should not be modified by the
        // pop operation src:
        // https://www.masswerk.at/6502/6502_instruction_set.html#PLP
        cpu->sr = (cpu->sr & (BREAK | UNUSED)) | (mem_pop_stack_8(cpu) & ~(BREAK | UNUSED));
        break;
    }
    case ROL: {
        if (instruction.address_mode == ACC)
            cpu->ac = rotate_left(cpu, cpu->ac);
        else {
            uint8_t m = mem_read_8(mem, cpu->address);
            mem_write_8(mem, cpu->address, rotate_left(cpu, m));
        }
        break;
    }
    case ROR: {
        if (instruction.address_mode == ACC)
            cpu->ac = rotate_right(cpu, cpu->ac);
        else {
            uint8_t m = mem_read_8(mem, cpu->address);
            mem_write_8(mem, cpu->address, rotate_right(cpu, m));
        }
        break;
    }
    case RTI: {
        cpu->sr = (cpu->sr & (BREAK | UNUSED)) | (mem_pop_stack_8(cpu) & ~(BREAK | UNUSED));
        cpu->pc = pop_stack_16(cpu);
        break;
    }
    case RTS: {
        // return the stored address and continue execution from the address
        // after it
        cpu->pc = pop_stack_16(cpu) + 1;
        break;
    }
    case SBC: {
        // Subtraction is addition of the two's complement of M
        uint16_t A = cpu->ac;
        uint16_t M = mem_read_8(mem, cpu->address);
        uint16_t R = A + (M ^ 0xFF) + get_flag(cpu, CARRY);
        cpu->ac = R & 0xFF;
        set_flag(cpu, CARRY, R > 0xFF);
        set_flag(cpu, OVERFLW, ((A ^ R) & (A ^ M) & 0x80) != 0);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case SEC: {
        set_flag(cpu, CARRY, TRUE);
        break;
    }
    case SED: {
        set_flag(cpu, DECIMAL, TRUE);
        break;
    }
    case SEI: {
        set_flag(cpu, INTERRUPT, TRUE);
        break;
    }
    case STA: {
        mem_write_8(mem, cpu->address, cpu->ac);
        break;
    }
    case STX: {
        mem_write_8(mem, cpu->address, cpu->x);
        break;
    }
    case STY: {
        mem_write_8(mem, cpu->address, cpu->y);
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

        // Illegal opcodes
    case ALR: { // Illegal
        // Perform AND
        cpu->ac &= mem_read_8(mem, cpu->address);
        cpu->ac = shift_right(cpu, cpu->ac);
        break;
    }
    case ANC: { // Illegal
        // Perform AND
        uint8_t m = mem_read_8(mem, cpu->address);
        cpu->ac &= m;
        set_ZN_flags(cpu, cpu->ac);
        set_flag(cpu, CARRY, cpu->ac & 0x80);
        break;
    }
    case AN2: { // Illegal
        // todo: implement AN2
        printf("AN2");
        exit(EXIT_FAILURE);
        break;
    }
    case ANE: { // Illegal
        // todo: implement ANE
        printf("ANE");
        exit(EXIT_FAILURE);
        break;
    }
    case ARR: { // Illegal
        uint8_t x = cpu->ac & mem_read_8(mem, cpu->address);
        uint8_t rotated = rotate_right(cpu, x);
        set_flag(cpu, CARRY, rotated & 0x40);
        set_flag(cpu, OVERFLW, ((rotated & 0x40) >> 1) ^ (rotated & 0x20));
        cpu->ac = rotated;
        break;
    }
    case DCP: { // Illegal
        // Perform DEC
        uint8_t m = mem_read_8(mem, cpu->address);
        m = (m - 1) & 0xFF;
        mem_write_8(mem, cpu->address, m);
        // Perform CMP
        uint8_t a = cpu->ac;
        set_flag(cpu, CARRY, a >= m);
        set_ZN_flags(cpu, a - m);
        break;
    }
    case ISB: { // Illegal
        // Perform INC
        uint8_t A = cpu->ac;
        uint8_t M = mem_read_8(mem, cpu->address) + 1;
        mem_write_8(mem, cpu->address, M);

        // Perform SBC
        uint16_t R = A + (M ^ 0xFF) + get_flag(cpu, CARRY);
        cpu->ac = R & 0xFF;
        set_flag(cpu, CARRY, R > 0xFF);
        set_flag(cpu, OVERFLW, ((A ^ R) & (A ^ M) & 0x80) != 0);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }

    case LAS: { // Illegal
        // todo: implement LAS
        printf("LAS");
        exit(EXIT_FAILURE);
        break;
    }
    case LAX: {                                  // Illegal
        cpu->ac = mem_read_8(mem, cpu->address); // Perform LDA
        cpu->x = cpu->ac;                        // Perform LDX
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case LXA: { // Illegal
        // todo: implement LXA
        printf("LXA");
        exit(EXIT_FAILURE);
        break;
    }
    case RLA: { // Illegal
        // Perform ROL
        uint8_t m = mem_read_8(mem, cpu->address);
        uint8_t rotated = (m << 1) | get_flag(cpu, CARRY);
        set_flag(cpu, CARRY, m & 0x80);
        mem_write_8(mem, cpu->address, rotated);
        // Perform AND
        cpu->ac &= rotated;
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case RRA: { // Illegal
        // Perform ROR
        uint8_t m = mem_read_8(mem, cpu->address);
        uint8_t rotated = (get_flag(cpu, CARRY) << 7) | (m >> 1);
        mem_write_8(mem, cpu->address, rotated);
        set_flag(cpu, CARRY, m & 0x01);
        // Perform ADC
        uint16_t a = cpu->ac;
        uint16_t r = a + rotated + get_flag(cpu, CARRY);
        cpu->ac = r & 0xFF;
        set_flag(cpu, CARRY, r >= 0x100);
        set_flag(cpu, OVERFLW, (~(a ^ rotated) & (a ^ r) & 0x80) != 0);
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case SAX: { // Illegal
        mem_write_8(mem, cpu->address, cpu->ac & cpu->x);
        break;
    }
    case SBX: { // Illegal
        uint8_t operand = mem_read_8(mem, cpu->address);
        uint16_t result = (cpu->ac & cpu->x) - operand;
        cpu->x = result;
        set_ZN_flags(cpu, cpu->x);
        set_flag(cpu, CARRY, !(result & 0xFF00));
        break;
    }
    case SHA: { // Illegal
        // todo: implement SHA
        printf("SHA");
        exit(EXIT_FAILURE);
        break;
    }
    case SHX: { // Illegal
        uint8_t hi = cpu->address >> 8;
        uint8_t lo = cpu->address & 0xff;
        uint8_t temp = cpu->x & (hi + 1);
        mem_write_8(mem, (temp << 8 | lo), temp);
        break;
    }
    case SHY: { // Illegal
        uint8_t hi = cpu->address >> 8;
        uint8_t lo = cpu->address & 0xff;
        uint8_t temp = cpu->y & (hi + 1);
        mem_write_8(mem, (temp << 8 | lo), temp);
        break;
    }
    case SLO: { // Illegal
        // Perform ASL
        uint8_t m = mem_read_8(mem, cpu->address);
        set_flag(cpu, CARRY, m & 0x80);
        uint8_t shifted = m << 1;
        mem_write_8(mem, cpu->address, shifted);
        // Perform ORA
        cpu->ac |= shifted;
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case SRE: { // Illegal
        // Perform LSR
        uint8_t m = mem_read_8(mem, cpu->address);
        set_flag(cpu, CARRY, m & 0x1);
        uint8_t shifted = m >> 1;
        mem_write_8(mem, cpu->address, shifted);
        // Perform EOR
        cpu->ac ^= shifted;
        set_ZN_flags(cpu, cpu->ac);
        break;
    }
    case TAS: { // Illegal
        // todo: implement TAS
        printf("TAS");
        exit(EXIT_FAILURE);
        break;
    }
    case UBC: { // Illegal
        // todo: implement UBC
        printf("UBC");
        exit(EXIT_FAILURE);
        break;
    }
    case JAM: { // Illegal
        // todo: implement JAM
        printf("JAM");
        exit(EXIT_FAILURE);
        break;
    }
    }
}

// This function sets up the cpu->address variable depending on the addressing
// mode It also updates cpu->cur_cycle if an indirect addressing mode crosses a
// page boundary
static void set_address(CPU *cpu, Instruction instruction) {
    MEM *mem = &cpu->emulator->mem;

    switch (instruction.address_mode) {
    case ACC: { // Accumulator
        break;
    }
    case ABS: { // Absolute
        cpu->address = mem_read_16(mem, cpu->pc);
        cpu->pc += 2;
        break;
    }
    case ABX: { // Absolute, X-indexed
        uint16_t base_address = mem_read_16(mem, cpu->pc);
        cpu->address = base_address + cpu->x;
        cpu->pc += 2;

        // If we cross page boundaries, we increment cur_cycle by 1.
        if (crosses_page_borders_(base_address, cpu->address, instruction.opcode))
            cpu->cycles++;
        break;
    }
    case ABY: { // Absolute, Y-indexed
        uint16_t base_address = mem_read_16(mem, cpu->pc);
        cpu->address = base_address + cpu->y;
        cpu->pc += 2;

        // If we cross page boundaries, we increment cur_cycle by 1.
        if (crosses_page_borders_(base_address, cpu->address, instruction.opcode))
            cpu->cycles++;
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
    case IND: { // Indirect
        uint16_t temp = mem_read_16(mem, cpu->pc);
        uint16_t indirect_address = mem_read_8(mem, temp) |
                                    (mem_read_8(mem, (temp & 0xFF00) | ((temp + 1) & 0xFF)) << 8);
        cpu->address = indirect_address;
        cpu->pc += 2;
        break;
    }
    case XIN: { // X-indexed, Indirect (Pre-Indexed Indirect)
        const uint8_t zp_address = (mem_read_8(mem, cpu->pc) + cpu->x) & 0xFF;
        uint16_t base_address = mem_read_8(mem, zp_address) | (mem_read_8(mem, (zp_address + 1) & 0xFF) << 8);
        cpu->address = base_address;
        cpu->pc++;
        break;
    }
    case YIN: { // Indirect, Y-indexed (Post-Indexed Indirect)
        const uint8_t zp_address = mem_read_8(mem, cpu->pc);
        uint16_t base_address = mem_read_8(mem, zp_address) | (mem_read_8(mem, (zp_address + 1) & 0xFF) << 8);
        cpu->address = base_address + cpu->y;
        cpu->pc++;

        // If we cross page boundaries, we increment cur_cycle by 1.
        if (crosses_page_borders_(base_address, cpu->address, instruction.opcode))
            cpu->cycles++;
        break;
    }
    case REL: { // Relative
        int8_t offset = (int8_t)mem_read_8(mem, cpu->pc);
        cpu->pc++;
        cpu->address = cpu->pc + offset;
        break;
    }
    case ZP0: { // Zeropage
        cpu->address = (uint16_t)mem_read_8(mem, cpu->pc);
        cpu->pc++;
        break;
    }
    case ZPX: { // Zeropage, X-indexed
        cpu->address = ((uint16_t)(mem_read_8(mem, cpu->pc) + cpu->x)) & 0xFF;
        cpu->pc++;
        break;
    }
    case ZPY: { // Zeropage, Y-indexed
        cpu->address = ((uint16_t)(mem_read_8(mem, cpu->pc) + cpu->y)) & 0xFF;
        cpu->pc++;
        break;
    }
    case UNK:
    default: // Unkown/Illegal
        // printf("Unknown Addressing Mode at PC: 0x%04X, Mode: %d\n", cpu->pc, instruction.address_mode);
        // exit(EXIT_FAILURE);
        break;
    }
}

void handle_interrupt(CPU *cpu) {
    if (cpu->pending_interrupt == NONE)
        return;

    if (get_flag(cpu, INTERRUPT) && cpu->pending_interrupt != NMI) {
        cpu->pending_interrupt = NONE;
        return;
    }

    MEM *mem = &cpu->emulator->mem;
    uint16_t address;

    switch (cpu->pending_interrupt) {
    case NMI: address = NMI_VECTOR_OFFSET; break;
    case IRQ: address = IRQ_VECTOR_OFFSET; break;
    case RSI:
        // TODO reset emulator
        printf("RESET INTERRUPT");
        exit(EXIT_FAILURE);
    default: printf("Error: invalid interrupt"); exit(EXIT_FAILURE);
    }

    mem_push_stack_16(cpu, cpu->pc);
    mem_push_stack_8(cpu, cpu->sr);
    set_flag(cpu, INTERRUPT, TRUE);
    cpu->pc = mem_read_16(mem, address);
    cpu->cycles += 7;
    cpu->pending_interrupt = NONE;
}