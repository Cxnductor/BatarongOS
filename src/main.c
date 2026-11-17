#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "memops.h"
#include "framebuf.h"
#include "keyboard.h"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_tsc_frequency_request tsc_request = {
    .id = LIMINE_TSC_FREQUENCY_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;

    asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));

    return ((uint64_t)hi << 32) | lo;
}

static uint64_t last_time = 0;

static uint64_t get_delta_cycles(void) {
    uint64_t current_time = rdtsc();

    uint64_t delta = current_time - last_time;
    last_time = current_time;

    return delta;
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    if (tsc_request.response == NULL ||
        tsc_request.response->frequency == 0) {
        hcf();
    }

    struct limine_framebuffer *framebuffer =
        framebuffer_request.response->framebuffers[0];

    if (framebuffer->memory_model != LIMINE_FRAMEBUFFER_RGB ||
        framebuffer->bpp != 32) {
        hcf();
    }

    
    last_time = rdtsc();

    
    keyboard_init();

    
    for (;;) {
        uint64_t delta_cycles = get_delta_cycles();
        uint64_t tsc_freq = tsc_request.response->frequency;

        
        keyboard_poll();

        
        static bool f1_was_pressed = false;
        bool f1_is_pressed = key_f1_pressed();
        if (f1_is_pressed && !f1_was_pressed) {
            reboot();
        }
        f1_was_pressed = f1_is_pressed;

        
        fb_pattern(framebuffer, delta_cycles, tsc_freq);

        
    }

    hcf();
}
