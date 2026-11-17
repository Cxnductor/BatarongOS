#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <stdbool.h>
#include <stddef.h>

void keyboard_init(void);
void keyboard_poll(void);
bool key_f1_pressed(void);
void reboot(void);

#endif
