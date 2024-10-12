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

typedef enum AddressMode {
    ACC, // Accumulator
    ABS, // Absolute
    ABX, // Absolute, X-indexed
    ABY, // Absolute, Y-indexed
    IMM, // Immediate
    IMP, // Implied
    IND, // Indirect
    XIN, // X-indexed, Indirect (Pre-Indexed Indirect)
    YIN, // Indirect, Y-indexed (Post-Indexed Indirect)
    REL, // Relative
    ZP0, // Zeropage
    ZPX, // Zeropage, X-indexed
    ZPY, // Zeropage, Y-indexed
    UNK, // Unkown/Illegal
} AddressMode;

typedef struct Instruction {
    Opcode opcode;
    AddressMode address_mode;
} Instruction;








//---------------//
// Lookup Tables //
//---------------//

static const Instruction instruction_lookup[] = {
    {BRK, IMP}, {ORA, XIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {ORA, ZP0}, {ASL, ZP0}, {ILL, UNK}, {PHP, IMP}, {ORA, IMM}, {ASL, ACC}, {ILL, UNK}, {ILL, UNK}, {ORA, ABS}, {ASL, ABS}, {ILL, UNK},
    {BPL, REL}, {ORA, YIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {ORA, ZPX}, {ASL, ZPX}, {ILL, UNK}, {CLC, IMP}, {ORA, ABY}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {ORA, ABX}, {ASL, ABX}, {ILL, UNK},
    {JSR, ABS}, {AND, XIN}, {ILL, UNK}, {ILL, UNK}, {BIT, ZP0}, {AND, ZP0}, {ROL, ZP0}, {ILL, UNK}, {PLP, IMP}, {AND, IMM}, {ROL, ACC}, {ILL, UNK}, {BIT, ABS}, {AND, ABS}, {ROL, ABS}, {ILL, UNK},
    {BMI, REL}, {AND, YIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {AND, ZPX}, {ROL, ZPX}, {ILL, UNK}, {SEC, IMP}, {AND, ABY}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {AND, ABX}, {ROL, ABX}, {ILL, UNK},
    {RTI, IMP}, {EOR, XIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {EOR, ZP0}, {LSR, ZP0}, {ILL, UNK}, {PHA, IMP}, {EOR, IMM}, {LSR, ACC}, {ILL, UNK}, {JMP, ABS}, {EOR, ABS}, {LSR, ABS}, {ILL, UNK},
    {BVC, REL}, {EOR, YIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {EOR, ZPX}, {LSR, ZPX}, {ILL, UNK}, {CLI, IMP}, {EOR, ABY}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {EOR, ABX}, {LSR, ABX}, {ILL, UNK},
    {RTS, IMP}, {ADC, XIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {ADC, ZP0}, {ROR, ZP0}, {ILL, UNK}, {PLA, IMP}, {ADC, IMM}, {ROR, ACC}, {ILL, UNK}, {JMP, IND}, {ADC, ABS}, {ROR, ABS}, {ILL, UNK},
    {BVS, REL}, {ADC, YIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {ADC, ZPX}, {ROR, ZPX}, {ILL, UNK}, {SEI, IMP}, {ADC, ABY}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {ADC, ABX}, {ROR, ABX}, {ILL, UNK},
    {ILL, UNK}, {STA, XIN}, {ILL, UNK}, {ILL, UNK}, {STY, ZP0}, {STA, ZP0}, {STX, ZP0}, {ILL, UNK}, {DEY, IMP}, {ILL, UNK}, {TXA, IMP}, {ILL, UNK}, {STY, ABS}, {STA, ABS}, {STX, ABS}, {ILL, UNK},
    {BCC, REL}, {STA, YIN}, {ILL, UNK}, {ILL, UNK}, {STY, ZPX}, {STA, ZPX}, {STX, ZPY}, {ILL, UNK}, {TYA, IMP}, {STA, ABY}, {TXS, IMP}, {ILL, UNK}, {ILL, UNK}, {STA, ABX}, {ILL, UNK}, {ILL, UNK},
    {LDY, IMM}, {LDA, XIN}, {LDX, IMM}, {ILL, UNK}, {LDY, ZP0}, {LDA, ZP0}, {LDX, ZP0}, {ILL, UNK}, {TAY, IMP}, {LDA, IMM}, {TAX, IMP}, {ILL, UNK}, {LDY, ABS}, {LDA, ABS}, {LDX, ABS}, {ILL, UNK},
    {BCS, REL}, {LDA, YIN}, {ILL, UNK}, {ILL, UNK}, {LDY, ZPX}, {LDA, ZPX}, {LDX, ZPY}, {ILL, UNK}, {CLV, IMP}, {LDA, ABY}, {TSX, IMP}, {ILL, UNK}, {LDY, ABX}, {LDA, ABX}, {LDX, ABY}, {ILL, UNK},
    {CPY, IMM}, {CMP, XIN}, {ILL, UNK}, {ILL, UNK}, {CPY, ZP0}, {CMP, ZP0}, {DEC, ZP0}, {ILL, UNK}, {INY, IMP}, {CMP, IMM}, {DEX, IMP}, {ILL, UNK}, {CPY, ABS}, {CMP, ABS}, {DEC, ABS}, {ILL, UNK},
    {BNE, REL}, {CMP, YIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {CMP, ZPX}, {DEC, ZPX}, {ILL, UNK}, {CLD, IMP}, {CMP, ABY}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {CMP, ABX}, {DEC, ABX}, {ILL, UNK},
    {CPX, IMM}, {SBC, XIN}, {ILL, UNK}, {ILL, UNK}, {CPX, ZP0}, {SBC, ZP0}, {INC, ZP0}, {ILL, UNK}, {INX, IMP}, {SBC, IMM}, {NOP, IMP}, {ILL, UNK}, {CPX, ABS}, {SBC, ABS}, {INC, ABS}, {ILL, UNK},
    {BEQ, REL}, {SBC, YIN}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {SBC, ZPX}, {INC, ZPX}, {ILL, UNK}, {SED, IMP}, {SBC, ABY}, {ILL, UNK}, {ILL, UNK}, {ILL, UNK}, {SBC, ABX}, {INC, ABX}, {ILL, UNK},
};

static const uint8_t cycle_lookup[] = {
    7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
};

static const char* opcode_name_lookup[] = {
    "ADC", "AND", "ASL", "BCC", "BCS",
    "BEQ", "BIT", "BMI", "BNE", "BPL",
    "BRK", "BVC", "BVS", "CLC", "CLD",
    "CLI", "CLV", "CMP", "CPX", "CPY",
    "DEC", "DEX", "DEY", "EOR", "INC",
    "INX", "INY", "JMP", "JSR", "LDA",
    "LDX", "LDY", "LSR", "NOP", "ORA",
    "PHA", "PHP", "PLA", "PLP", "ROL",
    "ROR", "RTI", "RTS", "SBC", "SEC",
    "SED", "SEI", "STA", "STX", "STY",
    "TAX", "TAY", "TSX", "TXA", "TXS",
    "TYA", "ILL"
};


// clang-format on
#endif
