/*
 * NES emulator compatible with iNES games without bank switching
 * (https://www.nesdev.org/wiki/INES)
 */
#include "cpu.h"
#include "load_rom.h"
#include "ppu.h"
#include "util.h"

// options
#define TESTING 0xC000 // entrypoint for nestest "automation mode" (comment
// out for normal entrypoint behavior)

// vector offsets
#define RESET_VECTOR_OFFSET 0xFFFC
#define NMI_VECTOR_OFFSET 0xFFFA
#define BREAKPOINT 0

// timing math

// NTSC https://www.emulationonline.com/systems/nes/nes-system-timing
// https://www.nesdev.org/wiki/Cycle_reference_chart
#define NTSC_CPU_CYCLES_PER_FRAME 29780
#define NTSC_CPU_CYCLES_PER_FRAME_VBLANK 2273
#define NTSC_CPU_CYCLES_PER_FRAME_ACTIVE                                       \
    (NTSC_CPU_CYCLES_PER_FRAME - NTSC_CPU_CYCLES_PER_FRAME_VBLANK)
#define NTSC_CPU_EXTRA_ACTIVE_CYCLE 2 // TODO: should be 3 if rendering is
// disabled during the 20th scanline
#define NTSC_CPU_EXTRA_VBLANK_CYCLE 3

// only needed for nestest
#define BOOTUP_SEQUENCE_CYCLES 0x07
#define SP_START 0x00FD

void new_frame(CPU *cpu, PPU *ppu) {

    // maybe TODO: implement true cycle accurate timing (draw scanlines instead
    // of frames)

    ppu_vblank_set(cpu->mem, FALSE);
    cpu_run_instructions(cpu, NTSC_CPU_CYCLES_PER_FRAME_ACTIVE +
                                  ppu->extra_cycle_active);

    // TODO: draw frame

    ppu_vblank_set(cpu->mem, TRUE);
    ppu_maybe_nmi(cpu);

    cpu_run_instructions(cpu, NTSC_CPU_CYCLES_PER_FRAME_VBLANK +
                                  ppu->extra_cycle_active);
}

// https://www.masswerk.at/6502/6502_instruction_set.html
void run_prg(CPU *cpu, PPU *ppu) {
    printf("Execution: (");

    uint16_t entrypoint = load_2_bytes(cpu->mem + RESET_VECTOR_OFFSET);

#ifdef TESTING
    entrypoint = TESTING;
    printf("breakpoint: 0x%04hX, ", BREAKPOINT);
    cpu->cur_cycle = BOOTUP_SEQUENCE_CYCLES;
#endif /* ifdef TESTING */

    printf("entrypoint: 0x%04hX)\n", entrypoint);

    cpu->pc = entrypoint;

    ppu->extra_cycle_active = 0;
    ppu->extra_cycle_vblank = 0;
    for (size_t frame = 0; TRUE; ++frame) {
        if (frame ==
            NTSC_CPU_EXTRA_ACTIVE_CYCLE * NTSC_CPU_EXTRA_VBLANK_CYCLE) {
            frame = 0; // to prevent overflow
        }
        if (frame % NTSC_CPU_EXTRA_ACTIVE_CYCLE == 0) {
            ppu->extra_cycle_active = 1;
        }
        if (frame % NTSC_CPU_EXTRA_VBLANK_CYCLE == 0) {
            ppu->extra_cycle_vblank = 1;
        }
        new_frame(cpu, ppu);
    }
}

int main(int argc, char *argv[]) {
    uint8_t *buffer;

    if (argc != 2) {
        printf("Fatal Error: No filepath provided\n");
        exit(EXIT_FAILURE);
    }

    size_t size = load_rom(&buffer, argv[1]); // size is needed to calculate the
    // misc roms section size for NES 2.0

    CPUMemory mem = {};
    CPU cpu = {.sp = SP_START, .mem = &mem};
    PPU ppu = {}; // partially initialize to zero all fields

    read_header_debug(buffer);
    static_memmap(buffer, cpu.mem, ppu.mem);
    free(buffer);

    // skip trainer for now
    run_prg(&cpu, &ppu);

    return EXIT_SUCCESS;
}
