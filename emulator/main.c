#include "emulator.h"
#include "load_rom.h"


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
    free(buffer); // We don't need buffer after initializing emulator


    // If --nestest option is specified we run nestest
    if (argc > 2 && strcmp(argv[2], "--nestest") == 0) {
        run_nestest(&NES);
    } else {
        run_emulator(&NES);
    }

#endif // !RISC_V

    return EXIT_SUCCESS;
}
