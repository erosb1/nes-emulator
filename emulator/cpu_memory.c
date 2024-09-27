#include "cpu_memory.h"

void init_memory(CPUMemory *mem) {
    memset(mem->ram, 0, RAM_SIZE);
    memset(mem->ppu_reg, 0, PPU_REGISTER_SIZE);
    memset(mem->apu_io_reg, 0, APU_IO_REGISTER_SIZE);
    memset(mem->cartridge_ram, 0, CARTRIDGE_RAM_SIZE);

    // Todo: Load in cartridge ROM from mapper
    memset(mem->cartridge_rom, 0, CARTRIDGE_ROM_SIZE);
}

void write_memory(CPUMemory *mem, uint16_t address, uint8_t value) {

    if (address < RAM_END) {
        mem->ram[address] = value;
        return;
    }

    if (address < RAM_MIRROR_END) {
        mem->ram[address % RAM_SIZE] = value; // Calculate corresponding address in RAM
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

    if (address < CARTRIDGE_RAM_END) {
        mem->cartridge_ram[address - APU_IO_REGISTER_END] = value;
        return;
    }

    // Todo, you should not be able to write to this region. This will be handled by mappers
    if (address < CARTRIDGE_ROM_END) {
        mem->cartridge_rom[address - CARTRIDGE_RAM_SIZE] = value;
        return;
    }

    printf("Tried to write to illegal memory address: %ui", address);
    assert(0);
}

uint8_t read_memory(CPUMemory *mem, uint16_t address) {
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

    if (address < CARTRIDGE_RAM_END) {
        return mem->cartridge_ram[address - APU_IO_REGISTER_END];
    }

    if (address < CARTRIDGE_ROM_END) {
        return mem->cartridge_rom[address - CARTRIDGE_RAM_SIZE];
    }

    printf("Tried to read from illegal memory address: %ui", address);
    assert(0);
    return 0;
}
