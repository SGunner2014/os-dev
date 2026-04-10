#ifndef INCLUDE_KBD_H
#define INCLUDE_KBD_H

#include "../idt.h"

struct key_flags {
    unsigned char caps;
    unsigned char lshift;
    unsigned char rshift;
    unsigned char lctrl;
    unsigned char rctrl;
};

void init_keyboard();

#endif
