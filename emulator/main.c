#include "emulator.h"
#include "load_rom.h"


int main(int argc, char *argv[]) {
    uint8_t *buffer;

#ifdef RISC_V
    load_rom(&buffer, NULL);
#else
    if (argc < 2) {
        printf("Fatal Error: No filepath provided\n");
        exit(EXIT_FAILURE);
    }
    load_rom(&buffer, argv[1]);
#endif // !RISC_V

    Emulator NES;
    init_emulator(&NES, buffer);

#ifdef RISC_V
    run_emulator(&NES);
#else

    // If --nestest option is specified we run nestest
    if (argc > 2 && strcmp(argv[2], "--nestest") == 0) {
        run_nestest(&NES);
    } else {
        run_emulator(&NES);
    }

    free(buffer);
#endif



    return EXIT_SUCCESS;
}
