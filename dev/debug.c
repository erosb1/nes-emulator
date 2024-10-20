
#include "common.h"
#include "cpu.h"
#include "emulator.h"
#include "mem.h"
#include "opcodes.h"
#include "util.h"
#include <assert.h>

extern SDLInstance SDL_INSTANCE;
extern WindowRegion NES_SCREEN;
extern WindowRegion DEBUG_SCREEN;

#define ADDRESS_MODE_COLUMN_WIDTH 28

static int is_illegal(uint8_t byte) {
    Instruction instruction = instruction_lookup[byte];
    return (instruction.opcode >= 56) ||                  // Illegal operand
           (instruction.opcode == NOP && byte != 0xEA) || // Illegal NOP
           byte == 0xEB;                                  // USBC (treated as SBC IMM)
}

/**
 *  This is a rather complex function that logs info about the addressing mode
 *  of the instruction that is currently about to be executed.
 *
 *  This function mimics the behavior of set_address in cpu.c,
 *  without actually updating the internal values of the cpu
 */
static void log_address_mode_info(const CPU *cpu, Instruction instruction) {
    const MEM *mem = &cpu->emulator->mem;
    uint8_t byte1 = mem_const_read_8(mem, cpu->pc + 1);
    uint8_t byte2 = mem_const_read_8(mem, cpu->pc + 2);
    size_t cur_column_width = 0;
    uint16_t address = 0x0000;

    switch (instruction.address_mode) {
    case ACC: {
        printf("A ");
        cur_column_width += 2;
        break;
    }
    case ABS: {
        address = (byte2 << 8) | byte1;
        printf("$%04X ", address);
        cur_column_width += 6;
        break;
    }
    case ABX: {
        uint16_t address_pre = (byte2 << 8) | byte1;
        address = address_pre + cpu->x;
        printf("$%04X,X @ %04X ", address_pre, address);
        cur_column_width += 15;
        break;
    }
    case ABY: {
        uint16_t address_pre = (byte2 << 8) | byte1;
        address = address_pre + cpu->y;
        printf("$%04X,Y @ %04X ", address_pre, address);
        cur_column_width += 15;
        break;
    }
    case IMM: {
        address = cpu->pc;
        printf("#$%02X ", byte1);
        cur_column_width += 5;
        break;
    }
    case IMP: {
        break;
    }
    case REL: {
        address = (cpu->pc + 2) + (int8_t)byte1;
        printf("$%04X ", address);
        cur_column_width += 6;
        break;
    }
    case IND: {
        uint16_t address_pre = (byte2 << 8) | byte1;
        address = mem_const_read_8(mem, address_pre) |
                  (mem_const_read_8(mem, (address_pre & 0xFF00) | ((address_pre + 1) & 0xFF)) << 8);
        printf("($%04X) = %04X ", address_pre, address);
        cur_column_width += 15;
        break;
    }
    case XIN: {
        uint16_t zp_address = (byte1 + cpu->x) & 0xFF;
        uint16_t hi_byte = mem_const_read_8(mem, (zp_address + 1) & 0xFF);
        uint16_t low_byte = mem_const_read_8(mem, zp_address & 0xFF);
        address = (hi_byte << 8) | low_byte;
        printf("($%02X,X) @ %02X = %04X ", byte1, zp_address, address);
        cur_column_width += 20;
        break;
    }
    case YIN: {
        uint16_t zp_address = byte1;
        uint16_t hi_byte = mem_const_read_8(mem, (zp_address + 1) & 0xFF);
        uint16_t low_byte = mem_const_read_8(mem, zp_address & 0xFF);
        uint16_t real_address = (hi_byte << 8) | low_byte;
        address = (real_address + cpu->y) & 0xFFFF;
        printf("($%02X),Y = %04X @ %04X ", byte1, real_address, address);
        cur_column_width += 22;
        break;
    }
    case ZP0: {
        address = byte1;
        printf("$%02X ", address);
        cur_column_width += 4;
        break;
    }
    case ZPX: {
        address = ((uint16_t)(byte1 + cpu->x)) & 0xFF;
        printf("$%02X,X @ %02X ", byte1, address);
        cur_column_width += 11;
        break;
    }
    case ZPY: {
        address = ((uint16_t)(byte1 + cpu->y)) & 0xFF;
        printf("$%02X,Y @ %02X ", byte1, address);
        cur_column_width += 11;
        break;
    }
    case UNK:
    default: {
        printf("???");
        cur_column_width += 3;
        break;
    }
    }

    // This is a rather ugly nested switch statement
    // Some instructions in the log show the value at the address it operates on.
    // This switch statement finds those instructions and prints the value.
    switch (instruction.address_mode) {
    case IMM:case ACC:case IND:case IMP: break;
    default:
        switch (instruction.opcode) {
        case STA:case STX:case STY:case BIT:case LDA:case LDX:case LDY:case CPY:
        case AND:case ORA:case EOR:case ADC:case SBC:case CMP:case CPX:case LSR:
        case ASL:case ROR:case ROL:case INC:case DEC:case NOP:case LAX:case SAX:
        case DCP:case ISB:case SLO:case RLA:case SRE:case RRA:
            printf("= %02X", mem_const_read_8(mem, address));
            cur_column_width += 4;
            break;
        default: break;
        }
    }

    // Print blank spaces so that the entire column width is equal to
    // ADDRESS_MODE_COLUMN_WIDTH
    for (int i = 0; i < ADDRESS_MODE_COLUMN_WIDTH - cur_column_width; i++) {
        printf(" ");
    }
}

void debug_log_instruction(const CPU *cpu) {
    const MEM *mem = &cpu->emulator->mem;
    const PPU *ppu = &cpu->emulator->ppu;
    uint8_t byte0 = mem_const_read_8(mem, cpu->pc);
    uint8_t byte1 = mem_const_read_8(mem, cpu->pc + 1);
    uint8_t byte2 = mem_const_read_8(mem, cpu->pc + 2);
    Instruction instruction = instruction_lookup[byte0];

    // Print the current PC
    printf("%04X  ", cpu->pc);

    // Print the bytes of the current instruction, for example: 4C F5 C5
    switch (instruction.address_mode) {
    case IMM: case ZP0: case ZPX: case ZPY:case XIN:case YIN: case REL: // Instruction is 2 bytes long
        printf("%02X %02X    ", byte0, byte1);
        break;
    case ABS: case ABX:case ABY:case IND: // Instruction is 3 bytes long
        printf("%02X %02X %02X ", byte0, byte1, byte2);
        break;
    default: // Instruction is 1 byte long
        printf("%02X       ", byte0);
    }

    // Illegal opcodes are prepended with a '*'
    if (is_illegal(byte0))
        printf("*");
    else
        printf(" ");

    // Print the name of the current instruction, for example: JMP
    printf("%s ", opcode_name_lookup[instruction.opcode]);

    log_address_mode_info(cpu, instruction);

    // Print the state of the CPU before the instruction is executed
    printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3li,%3li CYC:%lu",
        cpu->ac, cpu->x, cpu->y, cpu->sr, cpu->sp, ppu->cur_scanline, ppu->cur_dot, cpu->total_cycles);


    //printf(" Frame: %u", cpu->emulator->cur_frame);

    printf("\n");
}

static uint32_t get_color(uint8_t color_index) {
    switch (color_index) {
    case 1: return 0x555555;
    case 2: return 0xAAAAAA;
    case 3: return 0xFFFFFF;
    default: return 0x000000;
    }
}

static void draw_tile(const Emulator *emulator, uint16_t base_address, int tile_index, int left_coord, int top_coord) {
    assert (base_address == 0x0000 || base_address == 0x1000);

    const PPU *ppu = &emulator->ppu;

    // Calculate the address of the tile in VRAM
    uint16_t tile_address = base_address + (tile_index * TILE_BYTE_SIZE);

    for (int y = 0; y < TILE_HEIGHT; y++) {
        uint8_t low_byte = ppu_const_read_vram_data(ppu, tile_address + y);
        uint8_t high_byte = ppu_const_read_vram_data(ppu, tile_address + y + 8);
        for (int x = 0; x < TILE_HEIGHT; x++) {
            uint8_t low_bit = (low_byte >> (7 - x)) & 0x1;
            uint8_t high_bit = (high_byte >> (7 - x)) & 0x1;
            uint8_t color_index = (high_bit << 1) | low_bit;
            uint32_t rgb_color = get_color(color_index);
            sdl_put_pixel_region(&DEBUG_SCREEN, left_coord + x, top_coord + y, rgb_color);
        }
    }
}

static void draw_pattern_table(const Emulator *emulator, uint16_t base_address, int left_coord, int top_coord) {
    const int TILE_COUNT = PATTERN_TABLE_WIDTH * PATTERN_TABLE_HEIGHT;
    for (int tile_index = 0; tile_index < TILE_COUNT; tile_index++) {
        int tile_x = tile_index % PATTERN_TABLE_WIDTH;
        int tile_y = tile_index / PATTERN_TABLE_WIDTH;
        int tile_left_coord = left_coord + tile_x * TILE_WIDTH;
        int tile_top_coord = top_coord + tile_y * TILE_HEIGHT;
        draw_tile(emulator, base_address, tile_index, tile_left_coord, tile_top_coord);
    }
}

static void draw_nametable(const Emulator *emulator, uint16_t base_address, int left_coord, int top_coord) {
    assert(base_address == 0x2000 || base_address == 0x2400 || base_address == 0x2800 || base_address == 0x2C00);

    const PPU *ppu = &emulator->ppu;
    const uint16_t PATTERN_TABLE_BG_BASE_ADDRESS = ppu->control.pattern_background ? 0x1000 : 0x0000;

    // calculate end_address, exclude the attribute table
    uint16_t end_address = base_address + NAMETABLE_BYTE_SIZE - ATTRIBUTE_TABLE_BYTE_SIZE;

    for (uint16_t address = base_address; address < end_address; address++) {
        int nametable_index = address - base_address;
        int tile_x = nametable_index % NAMETABLE_WIDTH;
        int tile_y = nametable_index / NAMETABLE_WIDTH;
        int tile_left_coord = left_coord + tile_x * TILE_WIDTH;
        int tile_top_coord = top_coord + tile_y * TILE_HEIGHT;

        // read VRAM to get what tile to draw
        uint8_t tile_index = ppu_const_read_vram_data(ppu, address);

        // draw that tile
        draw_tile(emulator, PATTERN_TABLE_BG_BASE_ADDRESS, tile_index, tile_left_coord, tile_top_coord);
    }
}


void debug_draw_screen(const Emulator *emulator) {
    const Mapper *mapper = &emulator->mapper;

    for (int i = 0; i < NES_SCREEN_WIDTH; i++)
        for (int j = 0; j < NES_SCREEN_HEIGHT; j++)
            sdl_put_pixel_region(&NES_SCREEN, i, j, 0xFFFFFF);

    draw_pattern_table(emulator, 0x0000, 0, 0);
    draw_pattern_table(emulator, 0x1000, 128, 0);
    draw_nametable(emulator, 0x2000, 0, 128);

    uint16_t nametable_base_address_1 = 0x2000;
    uint16_t nametable_base_address_2 = 0x2400;
    if (mapper->mirroring == HORIZONTAL)
        nametable_base_address_2 = 0x2800;

    draw_nametable(emulator, nametable_base_address_1, 0, 128);
    draw_nametable(emulator, nametable_base_address_2, 0, 368);
}