#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "port.h"
#include <stddef.h>

// https://stackoverflow.com/questions/40961527/

#define	KEYBOARD_INTERRUPT 0x09 // The keyboard ISR number
#define MAKE_FAR_POINTER(segment, offset) ((void *)(((u32)(segment) << 16) | (u16)(offset)))
// #define FAR_POINTER_SEGMENT(far_pointer)  ((u16)(((u32)(far_pointer) >> 16) & 0xFFFF))
// #define FAR_POINTER_OFFSET(far_pointer)   ((u16)((u32)(far_pointer) & 0xFFFF))
#define FAR_POINTER_SEGMENT(ptr) ((unsigned short)(((unsigned long)(ptr)) >> 16))
#define FAR_POINTER_OFFSET(ptr)  ((unsigned short)((unsigned long)(ptr) & 0xFFFF))
void* get_interrupt_vector(volatile int intno) {
    unsigned short segment, offset;
    asm volatile(
        "mov $0x35, %%ah\n" // Get current interrupt AH
        "int $0x21\n" // Puts current interrupt AL in ES:BX
        "mov %%es, %0" // We can't read ES directly, so we move it to AX
        : "=r"(segment), "=b"(offset)
        : "Ral"((char)intno)
        : "ah"
    );
    return MAKE_FAR_POINTER(segment, offset);
}
// static inline void set_interrupt_vector(volatile int intno, volatile void *vect) {
//     asm volatile(
//         "mov %%dx, %%cx\n"
//         "int $0x21" // Sets the interrupt AL to DS:DX
//         : /* no output */
//         : "Ral"((char)intno), "Rds"(FAR_POINTER_SEGMENT(vect)), "d"(FAR_POINTER_OFFSET(vect))
//         : "ah"
//     );
// }
static inline void set_interrupt_vector(volatile int intno, volatile void *vect) {
    asm volatile (
        "cli\n"                              // Disable interrupts
        "push %%ds\n"

        // Load the segment and offset of the new interrupt handler
        "mov %1, %%ax\n"                     // Load new segment into AX
        "mov %%ax, %%es\n"                   // Set ES to the new segment (needed for addressing the IVT)
        "mov %2, %%ax\n"                     // Load new offset into AX

        // Calculate the address in the IVT (intno * 4 = segment:offset of interrupt handler)
        "shl $2, %%bx\n"                     // Multiply interrupt number by 4 (intno * 4)

        "mov 0, %%ax\n"                      // Load segment 0 (IVT segment) into AX
        "mov %%ax, %%ds\n"                   // Set DS to point to segment 0 (IVT base)

        // Store new offset and segment in the IVT
        "mov %%ax, %%ds:(%%bx)\n"            // Store offset at IVT[intno * 4]
        "mov %%es, %%ds:2(%%bx)\n"           // Store segment at IVT[intno * 4 + 2]

        "pop %%ds\n"
        "sti\n"                              // Re-enable interrupts
        : /* no output */
        : "b"((short)intno),                 // Input: interrupt number
          "r"(FAR_POINTER_SEGMENT(vect)),    // Input: new segment for the handler
          "r"(FAR_POINTER_OFFSET(vect))      // Input: new offset for the handler
        : "ax", "memory"
    );
}

unsigned char NORMAL_KEYS[0x60];
unsigned char EXTENDED_KEYS[0x60];

enum KEY_NAMES {
    KEY_ESC = 0x01, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE,
    KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LBRACKET, KEY_RBRACKET, KEY_ENTER,
    KEY_CTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_QUOTE, KEY_BACKTICK, KEY_LSHIFT,
    KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH, KEY_RSHIFT, KEY_PRINTSCREEN,
    KEY_ALT, KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    KEY_NUMLOCK, KEY_SCROLLLOCK, KEY_HOME, KEY_UP, KEY_PAGEUP, KEY_NUMPAD_MINUS, KEY_LEFT, KEY_NUMPAD_5, KEY_RIGHT, KEY_NUMPAD_PLUS, KEY_END, KEY_DOWN, KEY_PAGEDOWN,
    KEY_INSERT, KEY_DELETE, KEY_F11, KEY_F12, KEY_PAUSE,
    MOUSE_RIGHT = 0x7D, MOUSE_LEFT = 0x7E
};

static void __attribute__((interrupt)) keyboard_interrupt(void* _) {
    // TEMPORARY: Quit the program to show we recieved the interrupt
    asm("mov $0x4C, %ah\nint $0x21\n");

    // static unsigned char buffer;
    // unsigned char rawcode;
    // unsigned char make_break;
    // int scancode;

    // rawcode = inportb(0x60); /* read scancode from keyboard controller */
    // make_break = !(rawcode & 0x80); /* bit 7: 0 = make, 1 = break */
    // scancode = rawcode & 0x7F;

    // if(buffer == 0xE0) { /* second byte of an extended key */
    //     if(scancode < 0x60) {
    //         EXTENDED_KEYS[scancode] = make_break;
    //     }
    //     buffer = 0;
    // } else if(buffer >= 0xE1 && buffer <= 0xE2) {
    //     buffer = 0; /* ingore these extended keys */
    // } else if(rawcode >= 0xE0 && rawcode <= 0xE2) {
    //     buffer = rawcode; /* first byte of an extended key */
    // } else if(scancode < 0x60) {
    //     NORMAL_KEYS[scancode] = make_break;
    // }

    // outportb(0x20, 0x20); /* must send EOI to finish interrupt */
}
static void __attribute__((interrupt)) (*old_keyboard_interrupt)(void*);

void hook_keyboard_interrupt(void) {
    old_keyboard_interrupt = get_interrupt_vector(KEYBOARD_INTERRUPT);
    // print("Keyboard interrupt location: ");
    // printlong((unsigned long)old_keyboard_interrupt);
    // print("\n\r");
    // print("New keyboard interrupt location: ");
    // printlong((unsigned long)keyboard_interrupt);
    set_interrupt_vector(KEYBOARD_INTERRUPT, keyboard_interrupt);
}

void unhook_keyboard_interrupt(void) {
    if(old_keyboard_interrupt != NULL) {
        set_interrupt_vector(KEYBOARD_INTERRUPT, old_keyboard_interrupt);
        old_keyboard_interrupt = NULL;
    }
}

bool keyboard_is_key_down(enum KEY_NAMES key) {
    return NORMAL_KEYS[key];
}

#endif