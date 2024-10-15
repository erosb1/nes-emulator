#include "mapper.h"
#include "load_rom.h"
#include "emulator.h"


uint8_t nrom_read_prg(Mapper *mapper, uint16_t address) {
    if (mapper->prg_rom_size == 1) {
        // NROM-128: 16 KB PRG ROM mirrored at 0x8000-0xFFFF
        return mapper->prg_rom[address % 0x4000];
    }
    return mapper->prg_rom[address - 0x8000];
}

static void nrom_write_prg(const Mapper *mapper, uint16_t address, uint8_t value) {
}

void init_mapper(Emulator *emulator) {
    Mapper *mapper = &emulator->mapper;
    mapper->emulator = emulator;
    uint8_t *rom = emulator->rom;

    iNES_Header rom_header = read_iNES_header(rom);

    mapper->prg_rom_size = rom_header.prg_rom_size;
    mapper->prg_ram_size = rom_header.prg_ram_size;
    mapper->chr_rom_size = rom_header.chr_rom_size;

    mapper->mirroring = (rom_header.flags_6 & 0x08) ? 1 : 0;
    mapper->has_battery_backed_ram = (rom_header.flags_6 & 0x04) ? 1 : 0;
    mapper->has_trainer = (rom_header.flags_6 & 0x02) ? 1 : 0;
    mapper->four_screen = (rom_header.flags_6 & 0x01) ? 1 : 0;

    size_t trainer_offset = (rom_header.flags_6 & 0x04) ? 512 : 0;
    mapper->prg_rom = rom + 16 + trainer_offset;
    mapper->chr_rom = mapper->prg_rom + (rom_header.prg_rom_size * 16 * 1024);


    int mapper_num = (rom_header.flags_7 & 0xF0) | (rom_header.flags_6 >> 4);

    switch (mapper_num) {
    case NROM:
        mapper->read_prg = nrom_read_prg;
        //mapper->read_chr = nrom_read_chr;
        //mapper->write_chr = nrom_write_chr;
        break;
    case MMC1:
        printf("Error: Unsupported mapper: MMC1");
        exit(EXIT_FAILURE);
    case UXROM:
        printf("Error: Unsupported mapper: UXROM");
        exit(EXIT_FAILURE);
    case CNROM:
        printf("Error: Unsupported mapper: CNROM");
        exit(EXIT_FAILURE);
    case MMC3:
        printf("Error: Unsupported mapper: MMC3");
        exit(EXIT_FAILURE);
    default:
        printf("Error: Unsupported mapper: %i", mapper_num);
        exit(EXIT_FAILURE);
    }
}