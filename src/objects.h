#ifndef OBJECTS_H
#define OBJECTS_H

typedef struct Cube {
    Vec3 position;
    /// In degrees
    Vec3 rotation;
    i16 size;

    PaletteColor baseColor;
    u8 paletteIndex;
} Cube;

#endif