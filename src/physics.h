#ifndef PHYSICS_H
#define PHYSICS_H

#include "types.h"
#include "vec3.h"
#include "objects.h"

#define GRAVITY_FIXED_POINT TO_FIXED_POINT(-1)

/// In fixed-point, normalized
typedef Vec3 Axis;

typedef struct MinimumTranslationVector {
    Axis axis;
    /// In fixed point
    u16 value;
} MinimumTranslationVector;

typedef struct Projection {
    /// In fixed point
    u16 min;
    /// In fixed point
    u16 max;
} Projection;

Projection project(Cube* cube, Axis axis) {
    Vec3 corners_fp[8];
    get_cube_corners_fp(cube, corners_fp);

    u16 min = dot_product_fp(axis, corners_fp[0]);
    u16 max = min;
    for (int i = 1; i < 8; i++) {
        // NOTE: the axis must be normalized to get accurate projections
        u16 p = dot_product_fp(axis, corners_fp[i]);
        if(p < min) {
            min = p;
        } else if(p > max) {
            max = p;
        }
    }
    Projection result = {
        .min = min,
        .max = max
    };
    return result;
}

bool projection_intersects(Projection a, Projection b) {
    return a.min < b.max && a.max > b.min;
}

u16 get_overlap(Projection a, Projection b) {
    if(a.min < b.min) {
        return b.min - a.min;
    } else if (a.max > b.max) {
        return a.max - b.max;
    } else {
        return 0;
    }
}

void get_axes(Cube* cube, Axis axes[3]) {
    Vec3 corners_fp[8];
    get_cube_corners_fp(cube, corners_fp);

    for(int i = 0; i < 3; i++) {
        axes[i] = normalize_vector_fp(
            cross_product_fp(&corners_fp[0], &corners_fp[i + 1])
        );
    }
}

MinimumTranslationVector minimum_translation_vector(Cube* a, Cube* b) {
    u16 smallestOverlap = 65535;
    Axis smallestAxis = { 0, 0, 0 };

    Axis aAxes[3];
    get_axes(a, aAxes);
    for(int i = 0; i < 3; i++) {
        Projection aProjection = project(a, aAxes[i]);
        Projection bProjection = project(b, aAxes[i]);
        if(!projection_intersects(aProjection, bProjection)) {
            return (MinimumTranslationVector){0}; // We can't possibly overlap
        }
        u16 overlap = get_overlap(aProjection, bProjection);
        if(overlap < smallestOverlap) {
            smallestOverlap = overlap;
            smallestAxis = aAxes[i];
        }
    }

    Axis bAxes[3];
    get_axes(b, bAxes);
    for(int i = 0; i < 3; i++) {
        Projection aProjection = project(a, bAxes[i]);
        Projection bProjection = project(b, bAxes[i]);
        if(!projection_intersects(aProjection, bProjection)) {
            return (MinimumTranslationVector){0}; // We can't possibly overlap
        }
        u16 overlap = get_overlap(aProjection, bProjection);
        if(overlap < smallestOverlap) {
            smallestOverlap = overlap;
            smallestAxis = bAxes[i];
        }
    }

    if(smallestOverlap == 65535) {
        return (MinimumTranslationVector){0};
    }

    MinimumTranslationVector result = {
        .axis = smallestAxis,
        .value = smallestOverlap
    };
    return result;
}

void simulate(i16 deltaTime) {
    deltaTime = deltaTime;
    for(int a = 0; a < NUM_CUBES; a++) {
        for(int b = a + 1; b < NUM_CUBES; b++) {
            MinimumTranslationVector translation = minimum_translation_vector(&CUBES[a], &CUBES[b]);
            if(translation.value > 0) {
                CUBES[a].position.x += FROM_FIXED_POINT(FIXED_MUL(translation.axis.x, translation.value));
                CUBES[a].position.y += FROM_FIXED_POINT(FIXED_MUL(translation.axis.y, translation.value));
                CUBES[a].position.z += FROM_FIXED_POINT(FIXED_MUL(translation.axis.z, translation.value));
            }
        }
    }
}

#endif