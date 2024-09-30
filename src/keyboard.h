#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "port.h"
#include <stddef.h>

// https://stackoverflow.com/questions/40961527/

#define	KEYBOARD_INTERRUPT 0x09 // The keyboard ISR number

#define MAKE_FAR_POINTER(segment, offset) ((void *)(((u32)(segment) << 16) | (u16)(offset)))
#define FAR_POINTER_SEGMENT(far_pointer)  ((u16)(((u32)(far_pointer) >> 16) & 0xFFFF))
#define FAR_POINTER_OFFSET(far_pointer)   ((u16)((u32)(far_pointer) & 0xFFFF))
void* get_interrupt_vector(volatile int intno) {
    unsigned short segment, offset;
    asm volatile(
        "cli\n"
        "movb %2, %%al\n"
        "mov $0x35, %%ah\n" // AH for get current interrupt
        "int $0x21\n" // Puts current interrupt AL in ES:BX
        "mov %%es, %0\n" // We can't read ES directly, so we move it to AX
        "sti\n"
        : "=r"(segment), "=b"(offset)
        : "r"((char)intno)
        : "al", "ah"
    );
    return MAKE_FAR_POINTER(segment, offset);
}
static inline void set_interrupt_vector(volatile int intno, volatile void *vect) {
    asm volatile(
        "cli\n"
        "push %%ds\n"
        "mov $0x25, %%ah\n" // AH for set interrupt
        "movb %0, %%al\n"
        "mov %1, %%ds\n"
        "mov %2, %%dx\n"
        "int $0x21\n" // Sets the interrupt AL to DS:DX
        "pop %%ds\n"
        "sti\n"
        : /* no output */
        : "r"((char)intno), "r"(FAR_POINTER_SEGMENT(vect)), "r"(FAR_POINTER_OFFSET(vect))
        : "al", "ah", "dx"
    );
}

unsigned char KEYS_DOWN[0x60];

enum KEY_NAMES {
    KEY_ESC = 0x01, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE,
    KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LBRACKET, KEY_RBRACKET, KEY_ENTER,
    KEY_CTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_QUOTE, KEY_BACKTICK, KEY_LSHIFT,
    KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_RSHIFT, KEY_PRINTSCREEN,
    KEY_ALT, KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_NUMLOCK, KEY_SCROLLLOCK, KEY_HOME, KEY_UP, KEY_PAGEUP, KEY_NUMPAD_MINUS, KEY_LEFT, KEY_NUMPAD_5, KEY_RIGHT, KEY_NUMPAD_PLUS, KEY_END, KEY_DOWN, KEY_PAGEDOWN,
    KEY_INSERT, KEY_DELETE, KEY_F11, KEY_F12, KEY_PAUSE
};

static void (*old_keyboard_interrupt)(void*);
// static __attribute__((interrupt)) void keyboard_interrupt(void* sp) {
//     // u32 scancode = inportb(0x60);
//     // if(scancode != 0xE0) KEYS_DOWN[scancode & 0x7F] = ((scancode & 0x80) == 0);

//     // outportb(0x20, 0x20); /* must send EOI to finish interrupt */

//     // Call the old interrupt handler
//     if(old_keyboard_interrupt != NULL) {
//         old_keyboard_interrupt(sp);
//     }
// }
// The interrupt handler is in keyboard_interrupt.s
extern void keyboard_interrupt(void* sp);

void hook_keyboard_interrupt(void) {
    old_keyboard_interrupt = get_interrupt_vector(KEYBOARD_INTERRUPT);
    set_interrupt_vector(KEYBOARD_INTERRUPT, keyboard_interrupt);
}

void unhook_keyboard_interrupt(void) {
    if(old_keyboard_interrupt != NULL) {
        set_interrupt_vector(KEYBOARD_INTERRUPT, old_keyboard_interrupt);
        old_keyboard_interrupt = NULL;
    }
}

bool keyboard_is_key_down(enum KEY_NAMES key) {
    return KEYS_DOWN[key];
}

#endif