#ifndef PORT_H
#define PORT_H

#include "types.h"

static inline i16 inportb(i16 port) {
    volatile u16 value;
    asm volatile(
        "in %%dx, %%ax\n"
        : "=a"(value)
        : "d"(port)
    );
    return value;
}

static inline void outportb(i16 port, u8 value) {
    asm volatile(
        "out %%al, %%dx\n"
        : /* no output */
        : "d"(port), "a"(value)
    );
}

#endif
