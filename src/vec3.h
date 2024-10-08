#ifndef VEC3_H
#define VEC3_H

#include "math.h"

typedef struct Vec3 {
    i16 x;
    i16 y;
    i16 z;
} Vec3;

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

/// Rotates around the x axis, returning a fixed-point vector. The angle is a normal integer in degrees.
void rotate_x_fp(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);

    i32 y = point->y;
    i32 z = point->z;

    point->y = FIXED_MUL(y, cosA) - FIXED_MUL(z, sinA);
    point->z = FIXED_MUL(y, sinA) + FIXED_MUL(z, cosA);
}
/// Rotates around the y axis, returning a fixed-point vector. The angle is a normal integer in degrees.
void rotate_y_fp(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);
    
    i32 x = point->x;
    i32 z = point->z;

    point->x = FIXED_MUL(x, cosA) - FIXED_MUL(z, sinA);
    point->z = FIXED_MUL(x, sinA) + FIXED_MUL(z, cosA);
}
/// Rotates around the z axis, returning a fixed-point vector. The angle is a normal integer in degrees.
void rotate_z_fp(Vec3* point, i16 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);

    i32 x = point->x;
    i32 y = point->y;

    point->x = FIXED_MUL(x, cosA) - FIXED_MUL(y, sinA);
    point->y = FIXED_MUL(x, sinA) + FIXED_MUL(y, cosA);
}

// Returns the dot product of two integer vectors
static inline i16 dot_product(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
/// Returns the dot product of two fixed-point vectors
static inline i16 dot_product_fp(Vec3 a, Vec3 b) {
    return FIXED_MUL(a.x, b.x) + FIXED_MUL(a.y, b.y) + FIXED_MUL(a.z, b.z);
}
/// Returns the cross product of two fixed-point vectors
static inline Vec3 cross_product_fp(Vec3* a, Vec3* b) {
    Vec3 result;
    result.x = FIXED_MUL(a->y, b->z) - FIXED_MUL(a->z, b->y);
    result.y = FIXED_MUL(a->z, b->x) - FIXED_MUL(a->x, b->z);
    result.z = FIXED_MUL(a->x, b->y) - FIXED_MUL(a->y, b->x);
    return result;
}

static inline Vec3 vector_subtract(Vec3* a, Vec3* b) {
    return (Vec3){
        .x = a->x - b->x,
        .y = a->y - b->y,
        .z = a->z - b->z
    };
}

static inline Vec3 vector_add(Vec3* a, Vec3* b) {
    return (Vec3){
        .x = a->x + b->x,
        .y = a->y + b->y,
        .z = a->z + b->z
    };
}

static inline Vec3 vector_fp_scale(Vec3* a, i16 scale) {
    return (Vec3){
        .x = FIXED_MUL(a->x, scale),
        .y = FIXED_MUL(a->y, scale),
        .z = FIXED_MUL(a->z, scale)
    };
}


static inline i16 inverse_sqrt_fixed_point(i16 x) {
    i16 result = 0;
    for (int i = 0; i < 16; i++) {
        result = (result << 1) + (x >> (15 - i));
    }
    return result;
}

static inline Vec3 normalize_vector_fp(Vec3 vector) {
    u32 length = FIXED_MUL(vector.x, vector.x) + FIXED_MUL(vector.y, vector.y) + FIXED_MUL(vector.z, vector.z);
    i16 length_sqrt = fxrt_u16(length);
    return vector_fp_scale(&vector, length_sqrt);
}

#endif