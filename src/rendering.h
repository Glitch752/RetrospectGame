#ifndef RENDERING_H
#define RENDERING_H

#include "camera.h"
#include "objects.h"

#define CUBE_PALETTE_START 16

Vec3 projectPoint(Vec3 point) {
    Vec3 cameraPoint = transform_to_camera_space(point);

    i32 denominator = ((i32)cameraPoint.z + FOCAL_LENGTH) * 16;
    if (denominator <= 256) denominator = 256;
    i32 scale = (FOCAL_LENGTH * 256 * 256) / denominator;

    return (Vec3){
        .x = SCREEN_WIDTH / 2 + (i16)(((i32)cameraPoint.x * scale) / 256),
        .y = SCREEN_HEIGHT / 2 - (i16)(((i32)cameraPoint.y * scale) / 256),
        .z = cameraPoint.z
    };
}
void drawCube(Cube *cube) {
    Vec3 corners[8];
    get_cube_corners(cube, corners);
    
    Vec3 projectedCorners[8];
    for(i16 i = 0; i < 8; i++) {
        projectedCorners[i] = projectPoint(corners[i]);
    }
    
    draw_filled_triangle_with_depth(projectedCorners[0], projectedCorners[1], projectedCorners[2], cube->paletteIndex + 0);
    draw_filled_triangle_with_depth(projectedCorners[1], projectedCorners[2], projectedCorners[3], cube->paletteIndex + 0);
    draw_filled_triangle_with_depth(projectedCorners[0], projectedCorners[1], projectedCorners[4], cube->paletteIndex + 1);
    draw_filled_triangle_with_depth(projectedCorners[1], projectedCorners[4], projectedCorners[5], cube->paletteIndex + 1);
    draw_filled_triangle_with_depth(projectedCorners[0], projectedCorners[2], projectedCorners[4], cube->paletteIndex + 2);
    draw_filled_triangle_with_depth(projectedCorners[2], projectedCorners[4], projectedCorners[6], cube->paletteIndex + 2);
    draw_filled_triangle_with_depth(projectedCorners[4], projectedCorners[5], projectedCorners[6], cube->paletteIndex + 0);
    draw_filled_triangle_with_depth(projectedCorners[5], projectedCorners[6], projectedCorners[7], cube->paletteIndex + 0);
    draw_filled_triangle_with_depth(projectedCorners[2], projectedCorners[3], projectedCorners[6], cube->paletteIndex + 1);
    draw_filled_triangle_with_depth(projectedCorners[3], projectedCorners[6], projectedCorners[7], cube->paletteIndex + 1);
    draw_filled_triangle_with_depth(projectedCorners[1], projectedCorners[3], projectedCorners[5], cube->paletteIndex + 2);
    draw_filled_triangle_with_depth(projectedCorners[3], projectedCorners[5], projectedCorners[7], cube->paletteIndex + 2);
}

void drawGroundPlane() {
    i16 size = 400;

    Vec3 projectedCorners[4];
    projectedCorners[0] = (Vec3){ .x = 0, .y = 0, .z = 0 };
    projectedCorners[1] = (Vec3){ .x = 0, .y = 0, .z = size };
    projectedCorners[2] = (Vec3){ .x = size, .y = 0, .z = size };
    projectedCorners[3] = (Vec3){ .x = size, .y = 0, .z = 0 };

    for(int i = 0; i < 4; i++) {
        projectedCorners[i] = projectPoint(projectedCorners[i]);
    }

    draw_filled_triangle_with_depth(projectedCorners[0], projectedCorners[1], projectedCorners[2], 1);
    draw_filled_triangle_with_depth(projectedCorners[2], projectedCorners[3], projectedCorners[0], 1);
}

Vec3 rotateNormal(Cube *cube, Vec3 normal) {
    Vec3 rotatedNormal = normal;
    rotate_x(&rotatedNormal, cube->rotation.x);
    rotate_y(&rotatedNormal, cube->rotation.y);
    rotate_z(&rotatedNormal, cube->rotation.z);
    return rotatedNormal;
}
void shadeCube(Cube *cube) {
    Vec3 xNormal = rotateNormal(cube, (Vec3){ .x = 128, .y = 0, .z = 0 });
    Vec3 yNormal = rotateNormal(cube, (Vec3){ .x = 0, .y = 128, .z = 0 });
    Vec3 zNormal = rotateNormal(cube, (Vec3){ .x = 0, .y = 0, .z = 128 });

    Vec3 lightDirection = { .x = 128, .y = 128, .z = 128 };
    i16 lightIntensity = 256;
    i16 baseIntensity = 64;

    i16 xIntensity = abs(dot_product(xNormal, lightDirection)) / 128 * (lightIntensity - baseIntensity) / 256 + baseIntensity;
    i16 yIntensity = abs(dot_product(yNormal, lightDirection)) / 128 * (lightIntensity - baseIntensity) / 256 + baseIntensity;
    i16 zIntensity = abs(dot_product(zNormal, lightDirection)) / 128 * (lightIntensity - baseIntensity) / 256 + baseIntensity;

    // The dot products are the intensity of light (a multiplier of the base color) for each axis
    u8 paletteIndex = cube->paletteIndex;
    PaletteColor baseColor = cube->baseColor;
    write_palette(paletteIndex + 0, (PaletteColor){ baseColor.r * xIntensity / 256, baseColor.g * xIntensity / 256, baseColor.b * xIntensity / 256 });
    write_palette(paletteIndex + 1, (PaletteColor){ baseColor.r * yIntensity / 256, baseColor.g * yIntensity / 256, baseColor.b * yIntensity / 256 });
    write_palette(paletteIndex + 2, (PaletteColor){ baseColor.r * zIntensity / 256, baseColor.g * zIntensity / 256, baseColor.b * zIntensity / 256 });
}

#endif