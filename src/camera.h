#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"

#define NUM_CUBES 15
#define CUBE_PALETTE_START 16
#define FOCAL_LENGTH 4

static Vec3 CAMERA_POSITION = {0, 0, 0};

Vec3 transform_to_camera_space(Vec3 point) {
    Vec3 transformedPoint = point;

    transformedPoint.x -= CAMERA_POSITION.x;
    transformedPoint.y -= CAMERA_POSITION.y;
    transformedPoint.z -= CAMERA_POSITION.z;

    return transformedPoint;
}

static inline void move_camera(Vec3 movement) {
    CAMERA_POSITION.x += movement.x;
    CAMERA_POSITION.y += movement.y;
    CAMERA_POSITION.z += movement.z;
}

#endif