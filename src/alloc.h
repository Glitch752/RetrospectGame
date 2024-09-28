#ifndef ALLOC_H
#define ALLOC_H

#include "types.h"

extern char _heap;
static char *hbreak = &_heap;

static void *sbrk(u16 size) {
    char *ptr = hbreak;
    for(u16 i = 0; i < size; i++)
        ptr[i] = 0;
    hbreak += size;
    return ptr;
}

static void free(void) {
    hbreak = &_heap;
}

#endif
