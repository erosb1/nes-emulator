#include "mapper.h"
#include "emulator.h"

typedef struct {
    char magic[4];
    uint8_t prg_rom_size;
    uint8_t chr_rom_size;
    uint8_t flags_6;
    uint8_t flags_7;
    uint8_t prg_ram_size;
    uint8_t flags_9;
    uint8_t flags_10;
    uint8_t unused[5];
} iNES_Header;





// --------------- STATIC FORWARD DECLARATIONS ---------------- //
static iNES_Header read_iNES_header(const uint8_t *buffer);
static uint8_t nrom_read_prg(Mapper *mapper, uint16_t address);
static uint8_t nrom_read_chr(Mapper *mapper, uint16_t address);
static void nrom_write_chr_ram(Mapper *mapper, uint16_t address, uint8_t value);
static void set_nametable_mapping(Mapper *mapper, uint16_t top_left, uint16_t top_right, uint16_t bottom_left, uint16_t bottom_right);
static void set_mirroring(Mapper *mapper, Mirroring mirroring);



// --------------- PUBLIC FUNCTIONS --------------------------- //
void mapper_init(Emulator *emulator) {
    Mapper *mapper = &emulator->mapper;
    mapper->emulator = emulator;
    uint8_t *rom = emulator->rom;

    iNES_Header rom_header = read_iNES_header(rom);

    mapper->prg_rom_size = rom_header.prg_rom_size;
    mapper->prg_ram_size = rom_header.prg_ram_size;
    mapper->chr_rom_size = rom_header.chr_rom_size;

    mapper->has_battery_backed_ram = (rom_header.flags_6 & 0x04) ? 1 : 0;
    mapper->has_trainer = (rom_header.flags_6 & 0x02) ? 1 : 0;

    Mirroring mirroring;
    if (rom_header.flags_6 & 0x03) {
        mirroring = FOUR_SCREEN;
    } else if (rom_header.flags_6 & 0x01) {
        mirroring = VERTICAL;
    } else {
        mirroring = HORIZONTAL;
    }

    set_mirroring(mapper, mirroring);

    size_t trainer_offset = (rom_header.flags_6 & 0x04) ? 512 : 0;
    mapper->prg_rom = rom + 16 + trainer_offset;

    if (mapper->chr_rom_size > 0) {
        mapper->chr_rom = mapper->prg_rom + (rom_header.prg_rom_size * 16 * 1024);
    } else {
        mapper->chr_rom = mapper->chr_ram;
    }

    int mapper_num = (rom_header.flags_7 & 0xF0) | (rom_header.flags_6 >> 4);

    switch (mapper_num) {
    case NROM:
        mapper->read_prg = nrom_read_prg;
        mapper->read_chr = nrom_read_chr;
        mapper->write_chr = nrom_write_chr_ram;
        break;
    case MMC1: printf("Error: Unsupported mapper: MMC1"); exit(EXIT_FAILURE);
    case UXROM: printf("Error: Unsupported mapper: UXROM"); exit(EXIT_FAILURE);
    case CNROM: printf("Error: Unsupported mapper: CNROM"); exit(EXIT_FAILURE);
    case MMC3: printf("Error: Unsupported mapper: MMC3"); exit(EXIT_FAILURE);
    default: printf("Error: Unsupported mapper: %i", mapper_num); exit(EXIT_FAILURE);
    }


}


// --------------- STATIC FUNCTIONS --------------------------- //
static iNES_Header read_iNES_header(const uint8_t *buffer) {
    iNES_Header header;
    // Copy the first 16 bytes from the buffer into the iNES_Header struct
    for (int i = 0; i < sizeof(iNES_Header); i++) {
        ((uint8_t *)&header)[i] = buffer[i];
    }

    if (header.magic[0] != 'N' || header.magic[1] != 'E' || header.magic[2] != 'S' || header.magic[3] != 0x1A) {
        printf("Error when reading rom header: rom is not of type iNES");
        exit(EXIT_FAILURE);
    }

    return header;
}

static uint8_t nrom_read_prg(Mapper *mapper, uint16_t address) {
    if (mapper->prg_rom_size == 1) {
        // NROM-128: 16 KB PRG ROM mirrored at 0x8000-0xFFFF
        return mapper->prg_rom[address % 0x4000];
    }
    return mapper->prg_rom[address - 0x8000];
}

static uint8_t nrom_read_chr(Mapper *mapper, uint16_t address) { return mapper->chr_rom[address]; }

// static void nrom_write_prg(const Mapper *mapper, uint16_t address, uint8_t value) {}

static void nrom_write_chr_ram(Mapper *mapper, uint16_t address, uint8_t value) {
    mapper->chr_ram[address] = value;
}


void set_nametable_mapping(Mapper *mapper, uint16_t top_left, uint16_t top_right, uint16_t bottom_left, uint16_t bottom_right){
    mapper->nametable_map[0] = top_left;
    mapper->nametable_map[1] = top_right;
    mapper->nametable_map[2] = bottom_left;
    mapper->nametable_map[3] = bottom_right;
}

void set_mirroring(Mapper *mapper, Mirroring mirroring){
    switch (mirroring) {
    case HORIZONTAL:
        set_nametable_mapping(mapper, 0x0000, 0x0000, 0x0800, 0x0800);
        break;
    case VERTICAL:
        set_nametable_mapping(mapper, 0x0000, 0x0400, 0x0000, 0x0400);
        break;
    case FOUR_SCREEN:
        set_nametable_mapping(mapper, 0x0000, 0x0400, 0x0800, 0x0C00);
        break;
    default:
        set_nametable_mapping(mapper, 0, 0, 0, 0);
    }

    mapper->mirroring = mirroring;
}