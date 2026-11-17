#include <stdbool.h>
#include <stdint.h>

#include "keyboard.h"

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

#define KBD_DATA_PORT     0x60
#define KBD_STATUS_PORT   0x64
#define KBD_STAT_OUTPUT   0x01   
#define KBD_STAT_INPUT    0x02   

#define SCAN_F1           0x3B   
#define SCAN_RELEASE_BIT  0x80   

static bool key_state[128];

void keyboard_init(void) {
    for (size_t i = 0; i < 128; i++) {
        key_state[i] = false;
    }
}

void keyboard_poll(void) {
    
    while ((inb(KBD_STATUS_PORT) & KBD_STAT_OUTPUT) != 0) {
        uint8_t sc = inb(KBD_DATA_PORT);

        if (sc & SCAN_RELEASE_BIT) {
            uint8_t base = sc & 0x7F;
            if (base < 128) {
                key_state[base] = false;
            }
        } else if (sc < 128) {
            key_state[sc] = true;
        }
    }
}

bool key_f1_pressed(void) {
    return key_state[SCAN_F1];
}

void reboot(void) {
    
    uint8_t status;
    do {
        status = inb(KBD_STATUS_PORT);
    } while ((status & KBD_STAT_INPUT) != 0);

    
    
    outb(KBD_STATUS_PORT, 0xFE);

    
    for (;;) {
        asm volatile ("hlt");
    }
}
