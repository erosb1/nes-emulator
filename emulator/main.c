#include "emulator.h"



/*
 * Loads the ROM in the file specified by `path`
 * Stores it as a byte array in `buffer`
 *
 * This function is only available when compiling for development.
 *  i.e. for a regular computer and not the RISC-V board
 */
#ifndef RISC_V
size_t read_rom_from_file(uint8_t **buffer, const char *path) {
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
}
#endif






int main(int argc, char *argv[]) {

#ifdef RISC_V // This code will run on the DTEKV RISC-V board
    uint8_t *buffer = (uint8_t *)0x2000000;
    Emulator NES;
    init_emulator(&NES, buffer);
    run_emulator(&NES);

#else       // This code will run on a regular computer, i.e. one that has access to libc
    uint8_t *buffer;
    if (argc < 2) {
        printf("Fatal Error: No filepath provided\n");
        exit(EXIT_FAILURE);
    }
    read_rom_from_file(&buffer, argv[1]);
    Emulator NES;
    init_emulator(&NES, buffer);


    // If --nestest option is specified we run nestest
    if (argc > 2 && strcmp(argv[2], "--nestest") == 0) {
        run_nestest(&NES);
    } else {
        run_emulator(&NES);
    }

    free(buffer);
#endif // !RISC_V

    return EXIT_SUCCESS;
}
