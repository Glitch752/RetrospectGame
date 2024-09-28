#include "types.h"

static bool kbhit() {
    bool result;
    asm volatile("mov $1, %%ah\n"
                  "int $0x16\n"
                  "jnz set%=\n"
                  "mov $0, %0\n"
                  "jmp done%=\n"
                  "set%=:\n"
                  "mov $1, %0\n"
                  "done%=:\n"
                  : "=rm"(result));
    return result;
}

static u16 kb_read() {
    u16 key;
    asm volatile("mov $1, %%ah\n"
                  "int $0x16\n"
                  "jnz get%=\n"
                  "mov $0, %%ax\n"
                  "jmp done%=\n"
                  "get%=:\n"
                  "mov $0, %%ah\n"
                  "int $0x16\n"
                  "jmp done%=\n"
                  "done%=:\n"
                  : "=a"(key));
    return key;
}
