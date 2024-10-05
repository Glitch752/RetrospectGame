#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

void turn_on_mouse() {
    asm volatile( // Initialize mouse driver
        "mov   $0x00, %%ax\n"
        "int   $0x33\n"
        : /* no outputs */
        : /* no inputs */
        : "ax"
    );

    asm volatile( // Turn on mouse
        "mov   $0x01, %%ax\n"
        "int   $0x33\n"
        : /* no outputs */
        : /* no inputs */
        : "ax"
    );
}

void turn_off_mouse() {
    asm volatile( // Turn off mouse
        "mov   $0x02, %%ax\n"
        "int   $0x33\n"
        : /* no outputs */
        : /* no inputs */
        : "ax"
    );
}

typedef struct MouseState {
    /// 0 - left, 1 - right, 2 - center
    i16 button;
    i16 x;
    i16 y;
} MouseState;

MouseState get_mouse_state() {
    volatile i16 left, x, y;
    // Get status. BX = Mouse button pressed (1 - left, 2 - right, 3 - center), CX = X-coordinate, DX = Y-coordinate
    asm volatile(
        "mov   $0x03, %%ah\n"
        "int   $0x33\n"
        : "=b"(left), "=c"(x), "=d"(y)
        : /* no inputs */
        : "ah"
    );
    MouseState buttons = {
        .button = left,
        .x = x,
        .y = y
    };
    return buttons;
}


#endif