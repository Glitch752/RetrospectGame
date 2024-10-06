#ifndef OBJECTS_H
#define OBJECTS_H

#include "math.h"

typedef struct Cube {
    Vec3 position;
    /// In degrees
    Vec3 rotation;
    i16 size;

    Vec3 velocity;
    Vec3 acceleration;
    i16 mass;

    PaletteColor baseColor;
    u8 paletteIndex;
} Cube;

#endif