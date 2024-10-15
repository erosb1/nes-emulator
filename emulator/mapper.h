#ifndef MAPPER_H
#define MAPPER_H

#include "common.h"


enum {
    NROM  = 000,
    MMC1  = 001,
    UXROM = 002,
    CNROM = 003,
    MMC3  = 004,
};


typedef struct Emulator Emulator;

typedef struct Mapper {
    uint8_t *prg_rom;
    uint8_t *chr_rom;

    size_t prg_rom_size;
    size_t prg_ram_size;
    size_t chr_rom_size;

    uint8_t prg_bank[2];
    uint8_t chr_bank[8];

    uint8_t (*read_prg)(struct Mapper *mapper, uint16_t address);
    //void (*write_prg)(struct Mapper *mapper, uint16_t address, uint8_t value);
    uint8_t (*read_chr)(struct Mapper *mapper, uint16_t address);
    //void (*write_chr)(struct Mapper *mapper, uint16_t address, uint8_t value);

    uint8_t mirroring;                 // 0: vertical mirroring, 1: horizontal mirroring
    uint8_t has_battery_backed_ram;    // 1: Battery-backed PRG RAM present
    uint8_t has_trainer;               // 1: Trainer present in ROM
    uint8_t four_screen;

    Emulator *emulator;
} Mapper;


void init_mapper(Emulator *emulator);

#endif