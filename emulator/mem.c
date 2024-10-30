#include "mem.h"
#include "cpu.h"
#include "emulator.h"
#include "mapper.h"
#include "ppu.h"

void init_cpu_mem(Emulator *emulator) {
    MEM *mem = &emulator->mem;
    mem->emulator = emulator;

    mem->controller_shift_register = 0;

    memset(mem->ram, 0, sizeof(mem->ram));
    memset(mem->cartridge_ram, 0, sizeof(mem->cartridge_ram));
}

void mem_write_8(MEM *mem, uint16_t address, uint8_t value) {
    if (address < RAM_MIRROR_END) {
        mem->ram[address & 0x07FF] = value; // Handle RAM mirroring
        return;
    }

    if (address < PPU_MIRROR_END) {
        address = (address & 0x0007) + 0x2000;
        PPU *ppu = &mem->emulator->ppu;

        switch (address) {
        case 0x2000: // PPU_CONTROL
            ppu_set_ctrl(ppu, value);
            break;
        case 0x2001:
            // PPU_MASK
            ppu->mask.reg = value;
            break;
        case 0x2003: // OAM_ADDRESS
            ppu->oam_addr = value;
            break;
        case 0x2004: // OAM_DATA
            ppu->oam_data = value;
            break;
        case 0x2005: // PPU_SCROLL
            ppu_set_scroll(ppu, value);
            break;
        case 0x2006: // PPU_ADDR (vram address)
            ppu_set_vram_addr(ppu, value);
            break;
        case 0x2007: // PPU_DATA (vram data)
            ppu_write_vram_data(ppu, value);
            break;
        default: break;
        }
    }

    if (address < APU_IO_REGISTER_END) {
        switch (address) {
        case 0x4016:
#ifdef RISC_V
            // set latch pin
            set_pin(LATCH_PIN_MASK, value & 1);
#else
            mem->controller_shift_register = sdl_poll_events();
#endif
            break;
        }
        //mem->apu_io_reg[address - PPU_MIRROR_END] = value;
        return;
    }

    if (address < PRG_RAM_END) {
        mem->cartridge_ram[address - APU_IO_REGISTER_END] = value;
        return;
    }

    printf("Tried to write to illegal memory address: %04X", address);
    exit(EXIT_FAILURE);
}

uint8_t mem_read_8(MEM *mem, uint16_t address) {
    if (address < RAM_MIRROR_END) {
        return mem->ram[address & 0x07FF];
    }

    if (address < PPU_MIRROR_END) {
        address = (address & 0x0007) + 0x2000;
        PPU *ppu = &mem->emulator->ppu;

        switch (address) {
        case 0x2002: // PPU_STATUS
            return ppu_read_status(ppu);
        case 0x2004: // OAM_DATA
            return ppu->oam_data;
        case 0x2007: // PPU_DATA
            return ppu_read_vram_data(ppu);
        default:
            return 0x00; // This is returned when reading rom a WRITE_ONLY PPU
                         // register
        }
    }

    if (address < APU_IO_REGISTER_END) {
        switch (address) {
        case 0x4016: {
#ifdef RISC_V
            // pulse clock pin
            input_clock_pulse();
            return get_pin(DATA_PIN_MASK);
#else

            uint8_t data = (mem->controller_shift_register & 0x80) > 0;
            mem->controller_shift_register <<= 1;
            return data;
#endif
        }
        }
        //return mem->apu_io_reg[address - PPU_MIRROR_END];
    }

    if (address < PRG_RAM_END) {
        return mem->cartridge_ram[address - APU_IO_REGISTER_END];
    }

    // else
    Mapper *mapper = &mem->emulator->mapper;
    return mapper->read_prg(mapper, address);
}

void mem_write_16(MEM *mem, uint16_t address, uint16_t value) {
    mem_write_8(mem, address + 1, value >> 8);
    mem_write_8(mem, address, value);
}

uint16_t mem_read_16(MEM *mem, uint16_t address) {
    return (mem_read_8(mem, address + 1) << 8) | mem_read_8(mem, address);
}

void mem_push_stack_8(CPU *cpu, uint8_t value) {
    mem_write_8(&cpu->emulator->mem, STACK_OFFSET + cpu->sp, value);
    cpu->sp -= 1;
}

uint8_t mem_pop_stack_8(CPU *cpu) {
    cpu->sp += 1;
    uint16_t value = mem_read_8(&cpu->emulator->mem, STACK_OFFSET + cpu->sp);
    return value;
}

void mem_push_stack_16(CPU *cpu, uint16_t value) {
    mem_push_stack_8(cpu, (value >> 8) & 0xFF);
    mem_push_stack_8(cpu, value & 0xFF);
}

uint16_t pop_stack_16(CPU *cpu) {
    uint8_t low = mem_pop_stack_8(cpu);
    uint8_t high = mem_pop_stack_8(cpu);
    return (high << 8) | low;
}

uint8_t mem_const_read_8(const MEM *mem, uint16_t address) {
    if (address < RAM_MIRROR_END) {
        return mem->ram[address & 0x07FF];
    }

    if (address < PPU_MIRROR_END) {
        address = (address & 0x0007) + 0x2000;
        const PPU *ppu = &mem->emulator->ppu;

        switch (address) {
        case 0x2002: // PPU_STATUS
            return ppu->status.reg;
        case 0x2004: // OAM_DATA
            return ppu->oam_data;
        case 0x2007: // PPU_DATA
            return ppu_const_read_vram_data(ppu, address);
        default:
            return 0x00; // This is returned when reading rom a WRITE_ONLY PPU
            // register
        }
    }

    if (address < APU_IO_REGISTER_END) {
        return mem->apu_io_reg[address - PPU_MIRROR_END];
    }

    if (address < PRG_RAM_END) {
        return mem->cartridge_ram[address - APU_IO_REGISTER_END];
    }

    // else
    Mapper *mapper = &mem->emulator->mapper;
    return mapper->read_prg(mapper, address);
}
