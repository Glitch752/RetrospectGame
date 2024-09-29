#ifndef PRNG_H
#define PRNG_H

#include "types.h"

static u16 prng_next_u16(void) {
  static u16 x = 1, y = 1;
  u16 t = x ^ (x << 5);
  x = y;
  return y = (y ^ (y >> 1)) ^ (t ^ (t >> 3));
}
static u8 prng_next_u8(void) {
    return prng_next_u16() & 0xFF;
}

#endif