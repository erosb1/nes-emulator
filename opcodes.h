enum Opcode {
  BRK = 0x00,     // break / interrupt
  JMP_abs = 0x4C, // jump absolute
  SEI = 0x78,     // set interrupt disable
  LDX_imm = 0xA2, // load x immediate
  CLD = 0xD8,     // clear decimal
};
