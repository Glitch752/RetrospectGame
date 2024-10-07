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
static inline Vec3 crossProductFP(Vec3* a, Vec3* b) {
    Vec3 result;
    result.x = FIXED_MUL(a->y, b->z) - FIXED_MUL(a->z, b->y);
    result.y = FIXED_MUL(a->z, b->x) - FIXED_MUL(a->x, b->z);
    result.z = FIXED_MUL(a->x, b->y) - FIXED_MUL(a->y, b->x);
    return result;
}

void rotate_x(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);

    i32 y = TO_FIXED_POINT(point->y);
    i32 z = TO_FIXED_POINT(point->z);

    point->y = FROM_FIXED_POINT(FIXED_MUL(y, cosA) - FIXED_MUL(z, sinA));
    point->z = FROM_FIXED_POINT(FIXED_MUL(y, sinA) + FIXED_MUL(z, cosA));
}
void rotate_y(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);
    
    i32 x = TO_FIXED_POINT(point->x);
    i32 z = TO_FIXED_POINT(point->z);

    point->x = FROM_FIXED_POINT(FIXED_MUL(x, cosA) - FIXED_MUL(z, sinA));
    point->z = FROM_FIXED_POINT(FIXED_MUL(x, sinA) + FIXED_MUL(z, cosA));
}
void rotate_z(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);

    i32 x = TO_FIXED_POINT(point->x);
    i32 y = TO_FIXED_POINT(point->y);

    point->x = FROM_FIXED_POINT(FIXED_MUL(x, cosA) - FIXED_MUL(y, sinA));
    point->y = FROM_FIXED_POINT(FIXED_MUL(x, sinA) + FIXED_MUL(y, cosA));
}

static inline Vec3 vectorSubtract(Vec3* a, Vec3* b) {
    return (Vec3){
        .x = a->x - b->x,
        .y = a->y - b->y,
        .z = a->z - b->z
    };
}

static inline Vec3 vectorAdd(Vec3* a, Vec3* b) {
    return (Vec3){
        .x = a->x + b->x,
        .y = a->y + b->y,
        .z = a->z + b->z
    };
}

static inline Vec3 vectorFPScale(Vec3* a, i16 scale) {
    return (Vec3){
        .x = FIXED_MUL(a->x, scale),
        .y = FIXED_MUL(a->y, scale),
        .z = FIXED_MUL(a->z, scale)
    };
}

static inline Vec3 normalizeVectorFP(Vec3 vector) {
    i16 length = FIXED_MUL(vector.x, vector.x) + FIXED_MUL(vector.y, vector.y) + FIXED_MUL(vector.z, vector.z);
    length = FIXED_DIV(length, TO_FIXED_POINT(1));
    return vectorFPScale(&vector, length);
}

#endif