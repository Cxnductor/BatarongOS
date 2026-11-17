#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "memops.h"
#include "framebuf.h"
#include "keyboard.h"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = { 0xf9562b2d5c95a6c8, 0x6a7b384944536bdc, 6 };

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = { 0xf6b8f4b39de7d1ae, 0xfab91a6940fcb9cf, 0x785c6ed015d3e316, 0x181e920a7852b9d9 };

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = { 0xadc0e0531bb10d03, 0x9572709f31764c62 };

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
    if (limine_base_revision[2] == 0) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
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

        keyboard_poll();

        static bool f1_was_pressed = false;
        bool f1_is_pressed = key_f1_pressed();
        if (f1_is_pressed && !f1_was_pressed) {
            reboot();
        }
        f1_was_pressed = f1_is_pressed;

        fb_pattern(framebuffer, delta_cycles, 0);
    }

    hcf();
}
