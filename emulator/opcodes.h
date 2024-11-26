#ifndef OPCODES_H
#define OPCODES_H
// clang-format off

typedef enum Opcode{
    // Legal Opcodes
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
    TYA,


    // Illegal Opcodes
    ALR, ANC, AN2, ANE, ARR,
    DCP, ISB, LAS, LAX, LXA,
    RLA, RRA, SAX, SBX, SHA,
    SHX, SHY, SLO, SRE, TAS,
    UBC, JAM,
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
    UNK, // Unkown/Illegal (used with JAM)
} AddressMode;

typedef struct Instruction {
    Opcode opcode;
    AddressMode address_mode;
} Instruction;




//---------------//
// Lookup Tables //
//---------------//

static const Instruction instruction_lookup[] = {
    {BRK, IMP}, {ORA, XIN}, {JAM, UNK}, {SLO, XIN}, {NOP, ZP0}, {ORA, ZP0}, {ASL, ZP0}, {SLO, ZP0}, // 0x00 - 0x07
    {PHP, IMP}, {ORA, IMM}, {ASL, ACC}, {ANC, IMM}, {NOP, ABS}, {ORA, ABS}, {ASL, ABS}, {SLO, ABS}, // 0x08 - 0x0F
    {BPL, REL}, {ORA, YIN}, {JAM, UNK}, {SLO, YIN}, {NOP, ZPX}, {ORA, ZPX}, {ASL, ZPX}, {SLO, ZPX}, // 0x10 - 0x17
    {CLC, IMP}, {ORA, ABY}, {NOP, IMP}, {SLO, ABY}, {NOP, ABX}, {ORA, ABX}, {ASL, ABX}, {SLO, ABX}, // 0x18 - 0x1F
    {JSR, ABS}, {AND, XIN}, {JAM, UNK}, {RLA, XIN}, {BIT, ZP0}, {AND, ZP0}, {ROL, ZP0}, {RLA, ZP0}, // 0x20 - 0x27
    {PLP, IMP}, {AND, IMM}, {ROL, ACC}, {ANC, IMM}, {BIT, ABS}, {AND, ABS}, {ROL, ABS}, {RLA, ABS}, // 0x28 - 0x2F
    {BMI, REL}, {AND, YIN}, {JAM, UNK}, {RLA, YIN}, {NOP, ZPX}, {AND, ZPX}, {ROL, ZPX}, {RLA, ZPX}, // 0x30 - 0x37
    {SEC, IMP}, {AND, ABY}, {NOP, IMP}, {RLA, ABY}, {NOP, ABX}, {AND, ABX}, {ROL, ABX}, {RLA, ABX}, // 0x38 - 0x3F
    {RTI, IMP}, {EOR, XIN}, {JAM, UNK}, {SRE, XIN}, {NOP, ZP0}, {EOR, ZP0}, {LSR, ZP0}, {SRE, ZP0}, // 0x40 - 0x47
    {PHA, IMP}, {EOR, IMM}, {LSR, ACC}, {ALR, IMM}, {JMP, ABS}, {EOR, ABS}, {LSR, ABS}, {SRE, ABS}, // 0x48 - 0x4F
    {BVC, REL}, {EOR, YIN}, {JAM, UNK}, {SRE, YIN}, {NOP, ZPX}, {EOR, ZPX}, {LSR, ZPX}, {SRE, ZPX}, // 0x50 - 0x57
    {CLI, IMP}, {EOR, ABY}, {NOP, IMP}, {SRE, ABY}, {NOP, ABX}, {EOR, ABX}, {LSR, ABX}, {SRE, ABX}, // 0x58 - 0x5F
    {RTS, IMP}, {ADC, XIN}, {JAM, UNK}, {RRA, XIN}, {NOP, ZP0}, {ADC, ZP0}, {ROR, ZP0}, {RRA, ZP0}, // 0x60 - 0x67
    {PLA, IMP}, {ADC, IMM}, {ROR, ACC}, {ARR, IMM}, {JMP, IND}, {ADC, ABS}, {ROR, ABS}, {RRA, ABS}, // 0x68 - 0x6F
    {BVS, REL}, {ADC, YIN}, {JAM, UNK}, {RRA, YIN}, {NOP, ZPX}, {ADC, ZPX}, {ROR, ZPX}, {RRA, ZPX}, // 0x70 - 0x77
    {SEI, IMP}, {ADC, ABY}, {NOP, IMP}, {RRA, ABY}, {NOP, ABX}, {ADC, ABX}, {ROR, ABX}, {RRA, ABX}, // 0x78 - 0x7F
    {NOP, IMM}, {STA, XIN}, {NOP, IMM}, {SAX, XIN}, {STY, ZP0}, {STA, ZP0}, {STX, ZP0}, {SAX, ZP0}, // 0x80 - 0x87
    {DEY, IMP}, {NOP, IMM}, {TXA, IMP}, {ANE, IMM}, {STY, ABS}, {STA, ABS}, {STX, ABS}, {SAX, ABS}, // 0x88 - 0x8F
    {BCC, REL}, {STA, YIN}, {JAM, UNK}, {SHA, YIN}, {STY, ZPX}, {STA, ZPX}, {STX, ZPY}, {SAX, ZPY}, // 0x90 - 0x97
    {TYA, IMP}, {STA, ABY}, {TXS, IMP}, {TAS, ABY}, {SHY, ABX}, {STA, ABX}, {SHX, ABY}, {SHA, ABY}, // 0x98 - 0x9F
    {LDY, IMM}, {LDA, XIN}, {LDX, IMM}, {LAX, XIN}, {LDY, ZP0}, {LDA, ZP0}, {LDX, ZP0}, {LAX, ZP0}, // 0xA0 - 0xA7
    {TAY, IMP}, {LDA, IMM}, {TAX, IMP}, {LAX, IMM}, {LDY, ABS}, {LDA, ABS}, {LDX, ABS}, {LAX, ABS}, // 0xA8 - 0xAF
    {BCS, REL}, {LDA, YIN}, {JAM, UNK}, {LAX, YIN}, {LDY, ZPX}, {LDA, ZPX}, {LDX, ZPY}, {LAX, ZPY}, // 0xB0 - 0xB7
    {CLV, IMP}, {LDA, ABY}, {TSX, IMP}, {LAS, ABY}, {LDY, ABX}, {LDA, ABX}, {LDX, ABY}, {LAX, ABY}, // 0xB8 - 0xBF
    {CPY, IMM}, {CMP, XIN}, {NOP, IMM}, {DCP, XIN}, {CPY, ZP0}, {CMP, ZP0}, {DEC, ZP0}, {DCP, ZP0}, // 0xC0 - 0xC7
    {INY, IMP}, {CMP, IMM}, {DEX, IMP}, {SBX, IMM}, {CPY, ABS}, {CMP, ABS}, {DEC, ABS}, {DCP, ABS}, // 0xC8 - 0xCF
    {BNE, REL}, {CMP, YIN}, {JAM, UNK}, {DCP, YIN}, {NOP, ZPX}, {CMP, ZPX}, {DEC, ZPX}, {DCP, ZPX}, // 0xD0 - 0xD7
    {CLD, IMP}, {CMP, ABY}, {NOP, IMP}, {DCP, ABY}, {NOP, ABX}, {CMP, ABX}, {DEC, ABX}, {DCP, ABX}, // 0xD8 - 0xDF
    {CPX, IMM}, {SBC, XIN}, {NOP, IMM}, {ISB, XIN}, {CPX, ZP0}, {SBC, ZP0}, {INC, ZP0}, {ISB, ZP0}, // 0xE0 - 0xE7
    {INX, IMP}, {SBC, IMM}, {NOP, IMP}, {SBC, IMM}, {CPX, ABS}, {SBC, ABS}, {INC, ABS}, {ISB, ABS}, // 0xE8 - 0xEF
    {BEQ, REL}, {SBC, YIN}, {JAM, UNK}, {ISB, YIN}, {NOP, ZPX}, {SBC, ZPX}, {INC, ZPX}, {ISB, ZPX}, // 0xF0 - 0xF7
    {SED, IMP}, {SBC, ABY}, {NOP, IMP}, {ISB, ABY}, {NOP, ABX}, {SBC, ABX}, {INC, ABX}, {ISB, ABX}  // 0xF8 - 0xFF
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
    // Legal
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
    "TYA",

    // Illegal
    "ALR", "ANC", "AN2", "ANE", "ARR",
    "DCP", "ISB", "LAS", "LAX", "LXA",
    "RLA", "RRA", "SAX", "SBX", "SHA",
    "SHX", "SHY", "SLO", "SRE", "TAS",
    "UBC", "JAM"
};

// clang-format on
#endif
