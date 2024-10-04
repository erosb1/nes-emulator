#include "cpu_mem.h"
#include "cpu.h"
#include "util.h"

void cpu_write_mem_8(CPUMemory *mem, uint16_t address, uint8_t value) {
    if (address < RAM_END) {
        mem->ram[address] = value;
        return;
    }

    if (address < RAM_MIRROR_END) {
        mem->ram[address % RAM_SIZE] =
            value; // Calculate corresponding address in RAM
        return;
    }

    if (address < PPU_REGISTER_END) {
        mem->ppu_reg[address - RAM_MIRROR_END] = value;
        return;
    }

    if (address < PPU_MIRROR_END) {
        mem->ppu_reg[(address - RAM_MIRROR_END) % PPU_REGISTER_SIZE] = value;
        return;
    }

    if (address < APU_IO_REGISTER_END) {
        mem->apu_io_reg[address - PPU_MIRROR_END] = value;
        return;
    }

    if (address < PRG_RAM_END) {
        mem->cartridge_ram[address - APU_IO_REGISTER_END] = value;
        return;
    }

    // you should not be able to write to this region. This will be
    // handled by mappers
    // if (address < PRG_ROM_END) {
    //     mem->cartridge_rom[address - PRG_RAM_END] = value;
    //     return;
    // }

    printf("Tried to write to illegal memory address: %ui", address);
    assert(FALSE);
}

uint8_t cpu_read_mem_8(CPUMemory *mem, uint16_t address) {
    if (address < RAM_END) {
        return mem->ram[address];
    }

    if (address < RAM_MIRROR_END) {
        return mem->ram[address % RAM_SIZE];
    }

    if (address < PPU_REGISTER_END) {
        return mem->ppu_reg[address - RAM_MIRROR_END];
    }

    if (address < PPU_MIRROR_END) {
        return mem->ppu_reg[(address - RAM_MIRROR_END) % PPU_REGISTER_SIZE];
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
    uint16_t value = cpu_read_mem_16(cpu->mem, STACK_OFFSET + cpu->sp);
    cpu->sp += 1;
    return value;
}

void push_stack_16(CPU *cpu, uint16_t value) {
    cpu_write_mem_16(cpu->mem, STACK_OFFSET + cpu->sp, value);
    cpu->sp -= 2;
}

uint16_t pop_stack_16(CPU *cpu) {
    uint16_t value = cpu_read_mem_16(cpu->mem, STACK_OFFSET + cpu->sp);
    cpu->sp += 2;
    return value;
}
