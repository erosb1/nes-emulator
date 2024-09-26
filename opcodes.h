enum Opcode {
  BRK = 0x00,     // break / interrupt
  CLC = 0x18,     // clear carry
  JSR = 0x20,     // jump subroutine (absolute)
  SEC = 0x38,     // set carry
  JMP_abs = 0x4C, // jump (absolute)
  SEI = 0x78,     // set interrupt disable
  STX_zpg = 0x86, // store x (zero-page)
  BCC = 0x90,     // branch if not carry
  LDX_imm = 0xA2, // load x (immediate)
  LDA_imm = 0xA9, // load a (immediate)
  BCS = 0xB0,     // branch if carry (relative)
  BNE = 0xD0,     // branch if not equal zero (relative)
  CLD = 0xD8,     // clear decimal
  NOP = 0xEA,     // no operation
  BEQ = 0xF0      // branch on equal zero (relative)
};
