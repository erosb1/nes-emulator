#include "cpu_mem.h"
#include "cpu.h"
#include "util.h"

#include <ppu.h>

typedef enum {
    PPU_CTRL = 0x2000,
    PPU_MASK,
    PPU_STATUS,
    OAM_ADDR,
    OAM_DATA,
    PPU_SCROLL,
    VRAM_ADDR,
    VRAM_DATA,
} PPURegister;

void cpu_write_mem_8(CPUMemory *mem, uint16_t address, uint8_t value) {
    if (address < RAM_MIRROR_END) {
        mem->ram[address % RAM_SIZE] = value; // Handle RAM mirroring
        return;
    }

    if (address < PPU_MIRROR_END) {
        address = RAM_MIRROR_END + (address - RAM_MIRROR_END) % PPU_REGISTER_SIZE; // Handle PPU register mirroring
        PPU* ppu = mem->ppu;

        switch (address) {
        case PPU_CTRL:
            ppu->control.reg = value;
            break;
        case PPU_MASK:
            ppu->mask.reg = value;
            break;
        case PPU_STATUS:
            // PPU_STATUS is read only
            break;
        case OAM_ADDR:
            ppu->oam_addr = value;
            break;
        case OAM_DATA:
            ppu->oam_data = value;
            break;
        case PPU_SCROLL:
            if (ppu->write_toggle == 0) {
                ppu->scroll.bytes.low = value;
                ppu->write_toggle = 1;
            } else {
                ppu->scroll.bytes.high = value;
                ppu->write_toggle = 0;
            }
            break;
        case VRAM_ADDR:
            if (ppu->write_toggle == 0) {
                ppu->vram_addr.bytes.high = value;
                ppu->write_toggle = 1;
            } else {
                ppu->vram_addr.bytes.low = value;
                ppu->write_toggle = 0;
            }
            break;
        case VRAM_DATA:
            ppu->vram_data = value;
            break;
        default:
            break;
        }
    }

    if (address < APU_IO_REGISTER_END) {
        mem->apu_io_reg[address - PPU_MIRROR_END] = value;
        return;
    }

    if (address < PRG_RAM_END) {
        mem->cartridge_ram[address - APU_IO_REGISTER_END] = value;
        return;
    }

    printf("Tried to write to illegal memory address: %ui", address);
    assert(FALSE);
}

uint8_t cpu_read_mem_8(CPUMemory *mem, uint16_t address) {
    if (address < RAM_MIRROR_END) {
        return mem->ram[address % RAM_SIZE];
    }

    if (address < PPU_MIRROR_END) {
        address = RAM_MIRROR_END + (address - RAM_MIRROR_END) % PPU_REGISTER_SIZE; // Handle PPU register mirroring
        PPU* ppu = mem->ppu;

        switch (address) {
        case PPU_CTRL:
            return ppu->control.reg;
        case PPU_MASK:
            return ppu->mask.reg;
        case PPU_STATUS:
            return ppu->status.reg;
        case OAM_ADDR:
            return ppu->oam_addr;
        case OAM_DATA:
            return ppu->oam_data;
        case PPU_SCROLL:
            return 0x00; // Write Only (we return 0 when reading)
        case VRAM_ADDR:
            return 0x00; // Write Only (we return 0 when reading)
        case VRAM_DATA:
            return ppu->vram_data;
        default:
            return 0x00;
        }
    }

    if (address < APU_IO_REGISTER_END) {
        return mem->apu_io_reg[address - PPU_MIRROR_END];
    }

    if (address < PRG_RAM_END) {
        return mem->cartridge_ram[address - APU_IO_REGISTER_END];
    }

    // else
    return mem->cartridge_rom[address - PRG_RAM_END];
}

void cpu_write_mem_16(CPUMemory *mem, uint16_t address, uint16_t value) {
    cpu_write_mem_8(mem, address + 1, value >> BYTE_SIZE);
    cpu_write_mem_8(mem, address, value);
}

uint16_t cpu_read_mem_16(CPUMemory *mem, uint16_t address) {
    return (cpu_read_mem_8(mem, address + 1) << BYTE_SIZE) |
           cpu_read_mem_8(mem, address);
}

void push_stack_8(CPU *cpu, uint8_t value) {
    cpu_write_mem_8(cpu->mem, STACK_OFFSET + cpu->sp, value);
    cpu->sp -= 1;
}

uint8_t pop_stack_8(CPU *cpu) {
    cpu->sp += 1;
    uint16_t value = cpu_read_mem_8(cpu->mem, STACK_OFFSET + cpu->sp);
    return value;
}

void push_stack_16(CPU *cpu, uint16_t value) {
    push_stack_8(cpu, (value >> 8) & 0xFF);
    push_stack_8(cpu, value & 0xFF);
}

uint16_t pop_stack_16(CPU *cpu) {
    uint8_t low = pop_stack_8(cpu);
    uint8_t high = pop_stack_8(cpu);
    return (high << 8) | low;
}
