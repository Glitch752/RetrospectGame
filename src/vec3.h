#ifndef VEC3_H
#define VEC3_H

#include "math.h"

typedef struct Vec3 {
    i16 x;
    i16 y;
    i16 z;
} Vec3;

static inline i16 dot_product(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

void rotate_x(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);
    i32 y = point->y << 8;
    i32 z = point->z << 8;
    
    point->y = (((y * cosA) >> 8) - ((z * sinA) >> 8)) >> 8;
    point->z = (((y * sinA) >> 8) + ((z * cosA) >> 8)) >> 8;
}
void rotate_y(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);
    i32 x = point->x << 8;
    i32 z = point->z << 8;
    
    point->x = (((x * cosA) >> 8) + ((z * sinA) >> 8)) >> 8;
    point->z = (((-x * sinA) >> 8) + ((z * cosA) >> 8)) >> 8;
}
void rotate_z(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);
    i32 x = point->x << 8;
    i32 y = point->y << 8;
    
    point->x = (((x * cosA) >> 8) - ((y * sinA) >> 8)) >> 8;
    point->y = (((x * sinA) >> 8) + ((y * cosA) >> 8)) >> 8;
}

#endif