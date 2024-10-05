#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"
#include "graphics.h"

#define FOCAL_LENGTH 6

static Vec3 CAMERA_POSITION = {0, 0, 0};
static Point CAMERA_ROTATION = {0, 0};

Vec3 transform_to_camera_space(Vec3 point) {
    Vec3 transformedPoint = point;

    transformedPoint.x -= CAMERA_POSITION.x;
    transformedPoint.y -= CAMERA_POSITION.y;
    transformedPoint.z -= CAMERA_POSITION.z;

    rotate_y(&transformedPoint, CAMERA_ROTATION.y);
    rotate_x(&transformedPoint, CAMERA_ROTATION.x);

    return transformedPoint;
}

Vec3 move_camera(Vec3 direction) {
    rotate_y(&direction, -CAMERA_ROTATION.y);

    CAMERA_POSITION.x += direction.x;
    CAMERA_POSITION.y += direction.y;
    CAMERA_POSITION.z += direction.z;
}

#endif