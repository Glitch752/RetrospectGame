#include "print.h"
#include "graphics.h"
#include "time.h"
#include "alloc.h"
#include "mouse.h"
#include "keyboard.h"
#include "types.h"
#include "math.h"
#include "vec3.h"
#include "prng.h"
#include "objects.h"
#include "camera.h"
#include "rendering.h"

#define NUM_CUBES 16
Cube CUBES[NUM_CUBES];

#include "physics.h"

void init_CUBES() {
    for (i16 i = 0; i < 4; i++) {
        for (i16 j = 0; j < 4; j++) {
            i16 idx = i * 4 + j;
            CUBES[idx].position.x = i * 100 - 90;
            CUBES[idx].position.y = (i + j) % 2 * 50;
            CUBES[idx].position.z = j * 100 - 90;
            CUBES[idx].size = 50;
            CUBES[idx].rotation.x = idx * 30;
            CUBES[idx].rotation.y = 130 - idx * 10;
            CUBES[idx].rotation.z = 80 - idx * 10;
            CUBES[idx].baseColor = (PaletteColor){ prng_next_u8() % 32 + 32, prng_next_u8() % 32 + 32, prng_next_u8() % 32 + 32 };

            CUBES[idx].mass = TO_FIXED_POINT(10);
            CUBES[idx].velocity = (Vec3){ prng_next_u16() % 4 - 2, prng_next_u16() % 4 - 2, prng_next_u16() % 4 - 2 };
            CUBES[idx].acceleration = (Vec3){0, 0, 0};
            CUBES[idx].angularVelocity = (Vec3){ prng_next_u16() % 6 - 3, prng_next_u16() % 6 - 3, prng_next_u16() % 6 - 3 };
            CUBES[idx].angularAccel = (Vec3){0, 0, 0};
        }
    }
}

int _main(void) {
    hook_keyboard_interrupt();

    enter_13h_graphics_mode();
    clear_screen(0);

    graphics_text_print((struct Point){5, 5}, WHITE, "Hello, world!");

    init_CUBES();

    for(i16 i = 0; true; i++) {
        // Check controls
        if(keyboard_is_key_down(KEY_ESC)) {
            break;
        }

        if(keyboard_is_key_down(KEY_SPACE)) CAMERA_POSITION.y += 2;
        if(keyboard_is_key_down(KEY_LSHIFT)) CAMERA_POSITION.y -= 2;
        if(keyboard_is_key_down(KEY_A)) move_camera((Vec3){ -4, 0, 0 });
        if(keyboard_is_key_down(KEY_D)) move_camera((Vec3){ +4, 0, 0 });
        if(keyboard_is_key_down(KEY_W)) move_camera((Vec3){ 0, 0, +3 });
        if(keyboard_is_key_down(KEY_S)) move_camera((Vec3){ 0, 0, -3 });
        if(keyboard_is_key_down(KEY_E)) CAMERA_ROTATION.y += 1;
        if(keyboard_is_key_down(KEY_Q)) CAMERA_ROTATION.y -= 1;
        if(keyboard_is_key_down(KEY_Z)) CAMERA_ROTATION.x += 1;
        if(keyboard_is_key_down(KEY_X)) CAMERA_ROTATION.x -= 1;

        wait_for_vsync();
        clear_screen(0);

        graphics_text_print((struct Point){5, 5}, (i / 10) % 15 + 1, "Retrospect!");

        simulate(FIXED_DIV(TO_FIXED_POINT(1), TO_FIXED_POINT(50)));

        for (int i = 0; i < NUM_CUBES; i++) {
            // CUBES[i].rotation.x += 1;
            // CUBES[i].rotation.y += 1;
            // CUBES[i].rotation.z += 1;
            CUBES[i].paletteIndex = CUBE_PALETTE_START + i * 3;
            shadeCube(&CUBES[i]);
            drawCube(&CUBES[i]);
        }
    }

    clear_screen(0);

    enter_text_mode();

    print("Goodbye!\r\n");

    unhook_keyboard_interrupt();
    return 0;
}
