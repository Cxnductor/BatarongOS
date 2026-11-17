#ifndef FRAMEBUF_H_
#define FRAMEBUF_H_

uint32_t fb_channel(uint8_t value, uint8_t mask_size, uint8_t mask_shift);
uint32_t fb_pixel(struct limine_framebuffer *fb, uint8_t red, uint8_t green, uint8_t blue);
void fb_pattern(struct limine_framebuffer *fb, uint64_t delta_cycles, uint64_t tsc_freq);

#endif
