#include "emulator.h"
#include "util.h"
#include "gfx.h"

#define NTSC_FRAME_RATE 60
#define NTSC_CPU_CYCLES_PER_FRAME 29780

void init_emulator(Emulator *emulator, uint8_t *rom) {
    // Set the rom
    emulator->rom = rom;

    // Set the internal state
    emulator->event = 0;
    emulator->is_running = FALSE;
    emulator->cur_frame = 0;

    // Initialize components.
    init_cpu(emulator);
    init_ppu(emulator);
    init_cpu_mem(emulator);
    //init_ppu_mem(emulator);
    init_mapper(emulator);
}


void run_emulator(Emulator *emulator) {
    emulator->is_running = TRUE;
    CPU *cpu = &emulator->cpu;
    PPU *ppu = &emulator->ppu;

    // Frame loop
    while (emulator->is_running) {

        // Instruction accurate emulation
        while (cpu->cur_cycle < NTSC_CPU_CYCLES_PER_FRAME) {
            size_t cycle_before = cpu->cur_cycle;
            cpu_run_instruction(cpu);
            size_t cpu_instruction_cycle_count = cpu->cur_cycle - cycle_before;

            for (int i = 0; i < cpu_instruction_cycle_count * 3; i++) {
                ppu_run_cycle(ppu);
            }
        }

        poll_input(emulator);
        draw_frame();

        emulator->cur_frame++;
        if (emulator->cur_frame == 60)
            emulator->cur_frame = 0;
    }
}


void poll_input(Emulator *emulator){
#ifdef RISC_V
#else
    emulator->event = sdl_poll_events();
    if (emulator->event & WINDOW_QUIT) {
        emulator->is_running = FALSE;
    }
#endif
}



#define NESTEST_MAX_CYCLES 26554
#define NESTEST_START_CYCLE 7

void run_nestest(Emulator *emulator) {
    emulator->cpu.is_logging = 1;
    CPU *cpu = &emulator->cpu;
    cpu->cur_cycle = NESTEST_START_CYCLE;
    CPUMemory *mem = &emulator->cpu_mem;

    // These APU registers needs to be set to 0xFF at the start in order for nestest to complete
    cpu_write_mem_8(mem, 0x4004, 0xFF);
    cpu_write_mem_8(mem, 0x4005, 0xFF);
    cpu_write_mem_8(mem, 0x4006, 0xFF);
    cpu_write_mem_8(mem, 0x4007, 0xFF);
    cpu_write_mem_8(mem, 0x4015, 0xFF);

    while (cpu->cur_cycle <= NESTEST_MAX_CYCLES) {
        cpu_run_instruction(cpu);
    }
}

