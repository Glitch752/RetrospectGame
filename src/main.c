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

#define NUM_CUBES 9
Cube CUBES[NUM_CUBES];

#include "physics.h"

void init_CUBES() {
    for (i16 i = 0; i < 3; i++) {
        for (i16 j = 0; j < 3; j++) {
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

#define SPEED 4
#define ROTATION_SPEED 1

int _main(void) {
    hook_keyboard_interrupt();

    enter_13h_graphics_mode();
    clear_screen(0);

    graphics_text_print((struct Point){5, 5}, WHITE, "Initializing!");

    init_CUBES();

    bool controlsHintShown = true;
    bool controlsMenuShown = false;

    bool hPressedLastFrame = false;
    bool jPressedLastFrame = false;

    for(i16 i = 0; true; i++) {
        // Check controls
        if(keyboard_is_key_down(KEY_ESC)) {
            break;
        }

        if(keyboard_is_key_down(KEY_H)) {
            if(!hPressedLastFrame) controlsMenuShown = !controlsMenuShown;
            hPressedLastFrame = true;
        } else {
            hPressedLastFrame = false;
        }
        if(keyboard_is_key_down(KEY_J)) {
            if(!jPressedLastFrame) controlsHintShown = !controlsHintShown;
            jPressedLastFrame = true;
        } else {
            jPressedLastFrame = false;
        }

        if(keyboard_is_key_down(KEY_SPACE)) CAMERA_POSITION.y += SPEED;
        if(keyboard_is_key_down(KEY_LSHIFT)) CAMERA_POSITION.y -= SPEED;
        if(keyboard_is_key_down(KEY_A)) move_camera((Vec3){ -SPEED, 0, 0 });
        if(keyboard_is_key_down(KEY_D)) move_camera((Vec3){ +SPEED, 0, 0 });
        if(keyboard_is_key_down(KEY_W)) move_camera((Vec3){ 0, 0, +SPEED });
        if(keyboard_is_key_down(KEY_S)) move_camera((Vec3){ 0, 0, -SPEED });
        if(keyboard_is_key_down(KEY_E)) CAMERA_ROTATION.y += ROTATION_SPEED;
        if(keyboard_is_key_down(KEY_Q)) CAMERA_ROTATION.y -= ROTATION_SPEED;
        if(keyboard_is_key_down(KEY_Z)) CAMERA_ROTATION.x += ROTATION_SPEED;
        if(keyboard_is_key_down(KEY_X)) CAMERA_ROTATION.x -= ROTATION_SPEED;

        // simulate(FIXED_DIV(TO_FIXED_POINT(1), TO_FIXED_POINT(50)));

        wait_for_vsync();

        for (int i = 0; i < NUM_CUBES; i++) {
            CUBES[i].rotation.x += 1;
            CUBES[i].rotation.y += 1;
            CUBES[i].rotation.z += 1;

            CUBES[i].paletteIndex = CUBE_PALETTE_START + i * 3;
            shadeCube(&CUBES[i]);
            drawCube(&CUBES[i]);
        }

        drawGroundPlane();

        if(controlsHintShown) graphics_text_print((struct Point){5, 5}, 0, "H for controls and info");
        if(controlsHintShown) graphics_text_print((struct Point){4, 4}, 15, "H for controls and info");
        if(controlsMenuShown) {
            int y = 2;
            graphics_text_print((struct Point){2, y++ * 9}, 0, "This is a demo of 3D rasterization");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "using fixed-point math with DOS 13h");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "mode. There are some things currently");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "messed up, but I just wanted to get");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "something submitted before the deadline.");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "Sorry that it's pretty broken in places.");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "Notably, there's no near plane clipping");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "and the depth buffer is pretty wrong.");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "I implemented a physics simulation, but");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "I couldn't get it to work properly yet.");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "I'll try to update this when I can.");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "Controls:");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "  Move: WASD / space / left shift");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "  Pan camera: Q/E, pitch camera: Z/X");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "  Toggle controls hint visibility: J");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "  Show controls menu: H");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "  Exit: ESC");
            graphics_text_print((struct Point){2, y++ * 9}, 0, "Thanks for playing even if it's broken!");
            
            y = 2;
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "This is a demo of 3D rasterization");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "using fixed-point math with DOS 13h");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "mode. There are some things currently");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "messed up, but I just wanted to get");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "something submitted before the deadline.");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "Sorry that it's pretty broken in places.");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "Notably, there's no near plane clipping");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "and the depth buffer is pretty wrong.");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "I implemented a physics simulation, but");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "I couldn't get it to work properly yet.");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "I'll try to update this when I can.");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "Controls:");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "  Move: WASD / space / left shift");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "  Pan camera: Q/E, pitch camera: Z/X");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "  Toggle controls hint visibility: J");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "  Show controls menu: H");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "  Exit: ESC");
            graphics_text_print((struct Point){1, y++ * 9 - 1}, 15, "Thanks for playing even if it's broken!");
        }
        
        push_framebuffer();
    }

    clear_screen(0);

    enter_text_mode();

    print("Goodbye!\r\n");

    unhook_keyboard_interrupt();
    return 0;
}
