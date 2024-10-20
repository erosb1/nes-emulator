#include "emulator.h"
#include "timer.h"

#define NTSC_FRAME_RATE 60
#define NTSC_CPU_CYCLES_PER_FRAME 29780

// --------------- STATIC FORWARD DECLARATIONS ---------------- //
#ifndef RISC_V
static void handle_sdl(Emulator *emulator);
static void synchronize_frames(Emulator *emulator);
static uint32_t calculate_unsynced_fps(Emulator *emulator);
static uint32_t calculate_synced_fps(Emulator *emulator);
#endif


// --------------- PUBLIC FUNCTIONS ---------- ---------------- //
void emulator_init(Emulator *emulator, uint8_t *rom) {
    // Set the rom
    emulator->rom = rom;

    // Set the internal state
    emulator->event = 0;
    emulator->is_running = FALSE;
    emulator->cur_frame = 0;
    emulator->time_point_start = 0;
    memset(emulator->frame_times, 0, sizeof(emulator->frame_times));

    // Initialize components.
    ppu_init(emulator);
    init_cpu_mem(emulator);
    mapper_init(emulator);
    cpu_init(emulator);
}

void emulator_run(Emulator *emulator) {
    emulator->is_running = TRUE;
    CPU *cpu = &emulator->cpu;
    PPU *ppu = &emulator->ppu;

    // Frame loop
    while (emulator->is_running) {

        emulator->time_point_start = get_time_point();

        do {
            ppu_run_cycle(ppu);
            ppu_run_cycle(ppu);
            ppu_run_cycle(ppu);
            cpu_run_cycle(cpu);
        } while (!ppu->frame_complete);

        ppu->frame_complete = 0;
        cpu->total_cycles = 0;

#ifndef RISC_V
        handle_sdl(emulator);
#endif

        synchronize_frames(emulator);
    }
}

#define NESTEST_MAX_CYCLES 26554
#define NESTEST_START_CYCLE 7

void emulator_nestest(Emulator *emulator) {
    emulator->cpu.is_logging = 1;
    CPU *cpu = &emulator->cpu;
    PPU *ppu = &emulator->ppu;
    cpu->total_cycles = NESTEST_START_CYCLE;
    MEM *mem = &emulator->mem;
    cpu->pc = 0xC000;
    ppu->cur_dot = 18;

    // These APU registers needs to be set to 0xFF at the start in order for
    // nestest to complete
    mem_write_8(mem, 0x4004, 0xFF);
    mem_write_8(mem, 0x4005, 0xFF);
    mem_write_8(mem, 0x4006, 0xFF);
    mem_write_8(mem, 0x4007, 0xFF);
    mem_write_8(mem, 0x4015, 0xFF);

    do {
        ppu_run_cycle(ppu);
        ppu_run_cycle(ppu);
        ppu_run_cycle(ppu);
        cpu_run_cycle(cpu);
    } while (cpu->total_cycles <= NESTEST_MAX_CYCLES);
}


// --------------- STATIC FUNCTIONS --------------------------- //
#ifndef RISC_V
static void handle_sdl(Emulator *emulator) {
    sdl_draw_frame();
    if (sdl_window_quit())
        emulator->is_running = FALSE;

    // Clear screen and draw debug info only every 10th frame.
    // Otherwise the rendering gets to intensive
    if (emulator->cur_frame % 10 == 0) {
        sdl_clear_screen();
        debug_draw_screen(emulator);
    }

    if (emulator->cur_frame == 0) {
        uint32_t fps_synced = calculate_synced_fps(emulator);
        uint32_t fps_unsynced = calculate_unsynced_fps(emulator);
        char title[256];
        snprintf(title, sizeof(title), "NES Emulator - FPS: %u - UNSYNCED FPS: %u", fps_synced, fps_unsynced);
        sdl_set_window_title(title);
    }
}
#endif

void synchronize_frames(Emulator *emulator) {
    uint32_t time_point_end = get_time_point();
    uint32_t elapsed_us = get_elapsed_us(emulator->time_point_start, time_point_end);
    emulator->frame_times[emulator->cur_frame] = elapsed_us;

    emulator->cur_frame++;
    if (emulator->cur_frame == NTSC_FRAME_RATE) {
        emulator->cur_frame = 0;
    }

    // Sleep if the frame finished early
    if (elapsed_us < NTSC_FRAME_DURATION) {
        sleep_us(NTSC_FRAME_DURATION - elapsed_us);
    } else {
        // TODO: Handle lag - consider skipping next frame
    }
}

uint32_t calculate_unsynced_fps(Emulator *emulator) {
    double sum = 0;
    for (int i = 0; i < NTSC_FRAME_RATE; i++) {
        sum += (double) emulator->frame_times[i];
    }
    double average_frame_duration = sum / NTSC_FRAME_RATE / 1e6;

    if (average_frame_duration == 0) return 0;
    return (uint32_t) (1 / average_frame_duration);
}

uint32_t calculate_synced_fps(Emulator *emulator) {
    double sum = 0;
    for (int i = 0; i < NTSC_FRAME_RATE; i++) {
        uint32_t frame_time = emulator->frame_times[i];
        sum += frame_time < NTSC_FRAME_DURATION ? NTSC_FRAME_DURATION : frame_time;
    }
    double average_frame_duration = sum / NTSC_FRAME_RATE / 1e6;

    if (average_frame_duration == 0) return 0;
    return (uint32_t) (1 / average_frame_duration);
}