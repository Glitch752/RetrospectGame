asm("call  _main\n"
     "mov   $0x4C,%ah\n"
     "int   $0x21\n");

#include "print.h"
#include "graphics.h"
#include "time.h"
#include "alloc.h"
#include "keyboard.h"
#include "types.h"
#include "math.h"

typedef struct {
    i32 x;
    i32 y;
    i32 z;
} Vec3;

typedef struct {
    Vec3 position;
    /// In degrees
    Vec3 rotation;
    i32 size;
} Cube;

#define NUM_CUBES 10
#define FOCAL_LENGTH 1
#define FOV 90
Cube cubes[NUM_CUBES];

void initCubes() {
    for (int i = 0; i < NUM_CUBES; i++) {
        cubes[i].position.x = i * 10;
        cubes[i].position.y = i * 10;
        cubes[i].position.z = i * 10;
        cubes[i].size = 1000;
        cubes[i].rotation.x = 0;
        cubes[i].rotation.y = 0;
        cubes[i].rotation.z = 0;
    }
}

void rotateX(Vec3* point, i32 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);
    int y = point->y << 8;
    int z = point->z << 8;
    
    point->y = (y * cosA - z * sinA) >> 8;
    point->z = (y * sinA + z * cosA) >> 8;
}
void rotateY(Vec3* point, i32 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);
    int x = point->x << 8;
    int z = point->z << 8;
    
    point->x = (x * cosA + z * sinA) >> 8;
    point->z = (-x * sinA + z * cosA) >> 8;
}
void rotateZ(Vec3* point, i32 angle) {
    i32 cosA = cos_degrees(angle);
    i32 sinA = sin_degrees(angle);
    int x = point->x << 8;
    int y = point->y << 8;
    
    point->x = (x * cosA - y * sinA) >> 8;
    point->y = (x * sinA + y * cosA) >> 8;
}

void getCubeCorners(Cube cube, Vec3 corners[8]) {
    i32 halfSize = cube.size / 2;

    Vec3 localCorners[8] = {
        { -halfSize, -halfSize, -halfSize },
        {  halfSize, -halfSize, -halfSize },
        { -halfSize,  halfSize, -halfSize },
        {  halfSize,  halfSize, -halfSize },
        { -halfSize, -halfSize,  halfSize },
        {  halfSize, -halfSize,  halfSize },
        { -halfSize,  halfSize,  halfSize },
        {  halfSize,  halfSize,  halfSize }
    };

    // Rotate and translate each corner
    for(i32 i = 0; i < 8; i++) {
        rotateX(&localCorners[i], cube.rotation.x);
        rotateY(&localCorners[i], cube.rotation.y);
        rotateZ(&localCorners[i], cube.rotation.z);

        corners[i].x = cube.position.x + localCorners[i].x;
        corners[i].y = cube.position.y + localCorners[i].y;
        corners[i].z = cube.position.z + localCorners[i].z;
    }
}

struct Point projectPoint(Vec3 point) {
    i32 scale = (FOCAL_LENGTH << 8) / ((FOCAL_LENGTH << 8) + (point.z << 8));
    return (struct Point) {
        .x = SCREEN_WIDTH / 2 + (((point.x << 8) * scale) >> 8),
        .y = SCREEN_HEIGHT / 2 - (((point.y << 8) * scale) >> 8)
    };
}
void drawCube(Cube cube) {
    Vec3 corners[8];
    getCubeCorners(cube, corners);
    struct Point projectedCorners[8];
    for(i32 i = 0; i < 8; i++) {
        projectedCorners[i] = projectPoint(corners[i]);
    }

    // Draw lines between each corner
    for(i32 i = 0; i < 4; i++) {
        draw_line(projectedCorners[i], projectedCorners[(i + 1) % 4], WHITE);
        draw_line(projectedCorners[i + 4], projectedCorners[((i + 1) % 4) + 4], WHITE);
        draw_line(projectedCorners[i], projectedCorners[i + 4], WHITE);
    }
}

int _main(void) {
    enter_13h_graphics_mode();
    clear_screen(0);

    graphics_text_print((struct Point){5, 5}, WHITE, "Hello, world!");

    initCubes();

    for(i32 i = 0; true; i++) {
        wait_for_vsync();
        graphics_text_print((struct Point){i / 20, i / 20}, i % 512, "Hello, world!");

        for (int i = 0; i < NUM_CUBES; i++) {
            cubes[i].rotation.x += 1;
            cubes[i].rotation.y += 1;
            cubes[i].rotation.z += 1;
            drawCube(cubes[i]);
        }
    }

    return 0;
}
