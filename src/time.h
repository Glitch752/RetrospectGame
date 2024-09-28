#ifndef TIME_H
#define TIME_H

#include "types.h"

static u32 get_time() {
    u16 high, low;
    asm volatile("mov  $0x2C, %%ah\n"
                  "int  $0x21\n"
                  : "=c"(high), "=d"(low)
                  :
                  : "ah");
    return (((u32) high) << 16) | low;
}

#endif
