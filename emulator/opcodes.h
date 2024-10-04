#ifndef OPCODES_H
#define OPCODES_H
// clang-format off

typedef enum Opcode{
    ADC, AND, ASL, BCC, BCS,
    BEQ, BIT, BMI, BNE, BPL,
    BRK, BVC, BVS, CLC, CLD,
    CLI, CLV, CMP, CPX, CPY,
    DEC, DEX, DEY, EOR, INC,
    INX, INY, JMP, JSR, LDA,
    LDX, LDY, LSR, NOP, ORA,
    PHA, PHP, PLA, PLP, ROL,
    ROR, RTI, RTS, SBC, SEC,
    SED, SEI, STA, STX, STY,
    TAX, TAY, TSX, TXA, TXS,
    TYA, ILL
} Opcode;

typedef enum AddressingMode {
    NO, IMPL, ACC, REL, IMT,
    ZPG, ZPG_X, ZPG_Y, ABS, ABS_X,
    ABS_Y, IND, IND_IDX, IDX_IND,
} AddressingMode;

typedef struct Instruction {
    Opcode opcode;
    AddressingMode addressing_mode;
} Instruction;


// clang-format on
#endif
