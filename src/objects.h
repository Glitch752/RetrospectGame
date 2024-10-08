#ifndef OBJECTS_H
#define OBJECTS_H

#include "math.h"

typedef struct Cube {
    /// Not in fixed-point
    Vec3 position;
    /// Degrees, not in fixed-point
    Vec3 rotation;
    /// Not in fixed-point
    i16 size;

    /// In fixed-point
    Vec3 velocity;
    /// In fixed-point
    Vec3 acceleration;

    /// Degrees per second, in fixed-point
    Vec3 angularVelocity;
    /// Degrees per second squared, in fixed-point
    Vec3 angularAccel;

    /// In fixed-point
    i16 mass;

    /// The base color of the cube; r, g, and b are in the range [0, 63]
    PaletteColor baseColor;
    /// The palette index assigned to this cube; this, and the two next palette indices, are used to draw the cube
    u8 paletteIndex;
} Cube;

void get_cube_corners(Cube *cube, Vec3 corners[8]) {
    i16 halfSize = cube->size / 2;

    Vec3 localCorners[8] = {
        {  halfSize,  halfSize,  halfSize },
        {  halfSize,  halfSize, -halfSize },
        {  halfSize, -halfSize,  halfSize },
        {  halfSize, -halfSize, -halfSize },
        { -halfSize,  halfSize,  halfSize },
        { -halfSize,  halfSize, -halfSize },
        { -halfSize, -halfSize,  halfSize },
        { -halfSize, -halfSize, -halfSize },
    };

    // Rotate and translate each corner
    for(i16 i = 0; i < 8; i++) {
        rotate_x(&localCorners[i], cube->rotation.x);
        rotate_y(&localCorners[i], cube->rotation.y);
        rotate_z(&localCorners[i], cube->rotation.z);

        corners[i].x = cube->position.x + localCorners[i].x;
        corners[i].y = cube->position.y + localCorners[i].y;
        corners[i].z = cube->position.z + localCorners[i].z;
    }
}

void get_cube_corners_fp(Cube *cube, Vec3 corners[8]) {
    i16 halfSize = TO_FIXED_POINT(cube->size / 2);

    Vec3 localCorners[8] = {
        {  halfSize,  halfSize,  halfSize },
        {  halfSize,  halfSize, -halfSize },
        {  halfSize, -halfSize,  halfSize },
        {  halfSize, -halfSize, -halfSize },
        { -halfSize,  halfSize,  halfSize },
        { -halfSize,  halfSize, -halfSize },
        { -halfSize, -halfSize,  halfSize },
        { -halfSize, -halfSize, -halfSize },
    };

    // Rotate and translate each corner
    for(i16 i = 0; i < 8; i++) {
        rotate_x_fp(&localCorners[i], cube->rotation.x);
        rotate_y_fp(&localCorners[i], cube->rotation.y);
        rotate_z_fp(&localCorners[i], cube->rotation.z);

        corners[i].x = TO_FIXED_POINT(cube->position.x + localCorners[i].x);
        corners[i].y = TO_FIXED_POINT(cube->position.y + localCorners[i].y);
        corners[i].z = TO_FIXED_POINT(cube->position.z + localCorners[i].z);
    }
}

#endif