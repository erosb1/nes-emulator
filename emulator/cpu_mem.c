#include "cpu_mem.h"
#include "cpu.h"
#include "mapper.h"
#include "util.h"

#include <emulator.h>
#include <ppu.h>

void init_cpu_mem(Emulator *emulator) {
    CPUMemory *cpu_mem = &emulator->cpu_mem;
    cpu_mem->cpu = &emulator->cpu;
    cpu_mem->ppu = &emulator->ppu;
    cpu_mem->mapper = &emulator->mapper;
}

void cpu_write_mem_8(CPUMemory *mem, uint16_t address, uint8_t value) {
    if (address < RAM_MIRROR_END) {
        mem->ram[address % RAM_SIZE] = value; // Handle RAM mirroring
        return;
    }

    if (address < PPU_MIRROR_END) {
        address = RAM_MIRROR_END + (address - RAM_MIRROR_END) % PPU_REGISTER_SIZE; // Handle PPU register mirroring
        PPU* ppu = mem->ppu;

        switch (address) {
        case 0x2000: // PPU_CONTROL
            ppu->control.reg = value;
            break;
        case 0x2001: // PPU_MASK
            ppu->mask.reg = value;
            break;
        case 0x2003: // OAM_ADDRESS
            ppu->oam_addr = value;
            break;
        case 0x2004: // OAM_DATA
            ppu->oam_data = value;
            break;
        case 0x2005: // PPU_SCROLL
            break;
        case 0x2006: // PPU_ADDR (vram address)
            ppu_set_vram_addr(ppu, value);
            break;
        case 0x2007: // PPU_DATA (vram data)
            ppu_write_vram_data(ppu, value);
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
    exit(EXIT_FAILURE);
}

uint8_t cpu_read_mem_8(CPUMemory *mem, uint16_t address) {
    if (address < RAM_MIRROR_END) {
        return mem->ram[address % RAM_SIZE];
    }

    if (address < PPU_MIRROR_END) {
        address = RAM_MIRROR_END + (address - RAM_MIRROR_END) % PPU_REGISTER_SIZE; // Handle PPU register mirroring
        PPU* ppu = mem->ppu;

        switch (address) {
        case 0x2002: // PPU_STATUS
            return ppu_read_status(ppu);
        case 0x2004: // OAM_DATA
            return ppu->oam_data;
        case 0x2007: // PPU_DATA
            return ppu_read_vram_data(ppu);
        default:
            return 0x00; // This is returned when reading rom a WRITE_ONLY PPU register
        }
    }

    if (address < APU_IO_REGISTER_END) {
        return mem->apu_io_reg[address - PPU_MIRROR_END];
    }

    if (address < PRG_RAM_END) {
        return mem->cartridge_ram[address - APU_IO_REGISTER_END];
    }

    // else
    return mem->mapper->read_prg(mem->mapper, address);
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
    cpu_write_mem_8(cpu->cpu_mem, STACK_OFFSET + cpu->sp, value);
    cpu->sp -= 1;
}

uint8_t pop_stack_8(CPU *cpu) {
    cpu->sp += 1;
    uint16_t value = cpu_read_mem_8(cpu->cpu_mem, STACK_OFFSET + cpu->sp);
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
