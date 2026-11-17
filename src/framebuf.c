#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "batarong.h"

static uint32_t fb_channel(uint8_t value, uint8_t mask_size, uint8_t mask_shift) {
    uint64_t max = ((uint64_t)1 << mask_size) - 1;
    return (uint32_t)((value * max / 255) << mask_shift);
}

static uint32_t fb_pixel(struct limine_framebuffer *fb, uint8_t red, uint8_t green, uint8_t blue) {
    return fb_channel(red, fb->red_mask_size, fb->red_mask_shift)
         | fb_channel(green, fb->green_mask_size, fb->green_mask_shift)
         | fb_channel(blue, fb->blue_mask_size, fb->blue_mask_shift);
}

void fb_pattern(struct limine_framebuffer *fb, uint64_t delta_cycles, uint64_t tsc_freq) {
    volatile uint32_t *fb_ptr = fb->address;

    static uint64_t elapsed = 0;
    static size_t current_frame = 0;
    static int first_frame = 1;

    const uint32_t frame_count = gif_data[0];
    const uint32_t gif_width = gif_data[1];
    const uint32_t gif_height = gif_data[2];

    if (frame_count == 0)
        return;

    
    elapsed += delta_cycles;

    
    const uint32_t *frame = gif_data + 3;

    for (size_t i = 0; i < current_frame; i++) {
        frame += 2 + frame[1];
    }

    
    uint64_t frame_duration_cycles = (uint64_t)frame[0] * tsc_freq / 1000;

    
    int frame_advanced = 0;

    
    while (elapsed >= frame_duration_cycles) {
        elapsed -= frame_duration_cycles;
        current_frame = (current_frame + 1) % frame_count;
        frame_advanced = 1;

        
        frame = gif_data + 3;

        for (size_t i = 0; i < current_frame; i++) {
            frame += 2 + frame[1];
        }

        frame_duration_cycles = (uint64_t)frame[0] * tsc_freq / 1000;
    }

    
    if (!first_frame && !frame_advanced)
        return;

    first_frame = 0;

    
    const uint32_t *pixels = frame + 2;

    
    size_t offset_x = (fb->width > gif_width)
        ? (fb->width - gif_width) / 2 : 0;

    size_t offset_y = (fb->height > gif_height)
        ? (fb->height - gif_height) / 2 : 0;

    
    for (size_t y = 0; y < gif_height; y++) {
        size_t screen_y = offset_y + y;
        if (screen_y >= fb->height)
            continue;

        for (size_t x = 0; x < gif_width; x++) {
            size_t screen_x = offset_x + x;
            if (screen_x >= fb->width)
                continue;

            fb_ptr[screen_y * (fb->pitch / 4) + screen_x] =
                fb_pixel(fb, 0, 0, 0);
        }
    }

    
    for (size_t y = 0; y < gif_height; y++) {
        size_t screen_y = offset_y + y;
        if (screen_y >= fb->height)
            continue;

        for (size_t x = 0; x < gif_width; x++) {
            size_t screen_x = offset_x + x;
            if (screen_x >= fb->width)
                continue;

            uint32_t pixel = pixels[y * gif_width + x];

            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            fb_ptr[screen_y * (fb->pitch / 4) + screen_x] =
                fb_pixel(fb, r, g, b);
        }
    }
}
