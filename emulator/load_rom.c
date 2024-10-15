#include "load_rom.h"
#include "common.h"
#include "cpu_mem.h"
#include "ppu_mem.h"
#include "util.h"

// size units
#define PRG_SIZE_UNIT 0x4000 // 16 KiB
#define CHR_SIZE_UNIT 0x2000 // 8 KiB

// header parameters
#define HEADER_SIZE 0x10
#define PRG_SIZE_HEADER_IDX 0x04
#define CHR_SIZE_HEADER_IDX 0x05
#define FLAGS_6_HEADER_IDX 0x06
#define FLAGS_7_HEADER_IDX 0x07

// mappers
// nrom
// cpu
#define NROM_PRG_OFFSET_1 (0x8000 - PRG_RAM_END)
#define NROM_PRG_OFFSET_2 (0xC000 - PRG_RAM_END)
// ppu
#define NROM_CHR_OFFSET 0x0000

size_t load_rom(uint8_t **buffer, const char *path) {
#ifdef RISC_V
    *buffer = (uint8_t *)0x2000000;
    return 0x2000000; // arbitrary
#else
    FILE *fp;
    size_t expected_size;
    size_t actual_size;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("Fatal Error: Failed to open file %s\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END); // seek to end of file
    expected_size = ftell(fp);
    fseek(fp, 0, SEEK_SET); // seek to start of file
    *buffer = malloc(expected_size);

    actual_size = fread(*buffer, sizeof **buffer, expected_size, fp); // read
    fclose(fp);

    if (actual_size != expected_size) {
        printf("Fatal Error: Expected size (%zu) != actual size (%zu) %s\n",
               expected_size, actual_size, path);
        exit(EXIT_FAILURE);
    }
    return actual_size;

#endif /* ifdef RISC_V */
}


NESType detect_nes_file_type(const uint8_t *buffer) {
    // Check for iNES/NES 2.0 format (first 4 bytes should be "NES\x1A")
    if (strncmp((const char *)buffer, "NES\x1A", 4) == 0) {
        // Check for NES 2.0 format in byte 7
        if ((buffer[7] & 0x0C) == 0x08) {
            return NES_NES20;  // NES 2.0
        }
        return NES_INES;  // Standard iNES
    }

    // Check for UNIF format (first 4 bytes should be "UNIF")
    if (strncmp((const char *)buffer, "UNIF", 4) == 0) {
        return NES_UNIF;  // UNIF format
    }

    // Check for FDS format (first 4 bytes should be "FDS\x1A")
    if (strncmp((const char *)buffer, "FDS\x1A", 4) == 0) {
        return NES_FDS;  // Famicom Disk System format
    }

    return NES_UNKNOWN;
}




iNES_Header read_iNES_header(const uint8_t *buffer) {
    iNES_Header header;
    // Copy the first 16 bytes from the buffer into the iNES_Header struct
    memcpy(&header, buffer, sizeof(iNES_Header));
    return header;
}


void static_memmap(uint8_t *buffer, CPUMemory *cpu_mem, PPUMemory *ppu_mem) {
    size_t prg_size = buffer[PRG_SIZE_HEADER_IDX];
    size_t prg_size_bytes = prg_size * PRG_SIZE_UNIT;
    uint8_t chr_size = buffer[CHR_SIZE_HEADER_IDX];
    uint8_t chr_size_bytes = chr_size * CHR_SIZE_UNIT;

    uint8_t mapper_num = (buffer[FLAGS_6_HEADER_IDX] >> NIBBLE_SIZE) |
                         (buffer[FLAGS_7_HEADER_IDX] & NIBBLE_HI_MASK);

    buffer += HEADER_SIZE;

    switch (mapper_num) {
    case 0: {
        // #000 (nrom)
        memcpy(cpu_mem->cartridge_rom + NROM_PRG_OFFSET_1, buffer,
               prg_size_bytes);
        memcpy(ppu_mem->cartridge_rom + NROM_CHR_OFFSET,
               buffer + prg_size_bytes, chr_size_bytes);
        if (prg_size == 1) {
            memcpy(cpu_mem->cartridge_rom + NROM_PRG_OFFSET_2, buffer,
                   prg_size_bytes);
            break;
        }
        if (prg_size == 2) {
            memcpy(cpu_mem->cartridge_rom + NROM_PRG_OFFSET_2,
                   buffer + PRG_SIZE_UNIT, prg_size_bytes);
            break;
        }
    }
    default: {
        printf("Fatal Error: Bank switching not yet implemented\n");
        assert(FALSE);
    }
    }
}
