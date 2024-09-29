asm(
    "call  _main\n"
    "mov   $0x4C,%ah\n" // 4C is the DOS exit call
    "int   $0x21\n"
);

#include "print.h"
#include "graphics.h"
#include "time.h"
#include "alloc.h"
#include "keyboard.h"
#include "types.h"
#include "math.h"
#include "vec3.h"
#include "prng.h"
#include "objects.h"
#include "camera.h"
#include "rendering.h"

Cube CUBES[NUM_CUBES];

void init_CUBES() {
    for (int i = 0; i < NUM_CUBES; i++) {
        CUBES[i].position.x = i * 75 - 90;
        // CUBES[i].position.x = 0;
        CUBES[i].position.y = 0;
        CUBES[i].position.z = i * 30 + 60;
        CUBES[i].size = 50;
        CUBES[i].rotation.x = i * 30;
        CUBES[i].rotation.y = 130 - i * 10;
        CUBES[i].rotation.z = 80 - i * 10;
        CUBES[i].baseColor = (PaletteColor){ prng_next_u8() % 32 + 32, prng_next_u8() % 32 + 32, prng_next_u8() % 32 + 32 };
    }
}

int _main(void) {
    hook_keyboard_interrupt();

    // Temporary: loop forever
    // for(;;);

    enter_13h_graphics_mode();
    clear_screen(0);

    graphics_text_print((struct Point){5, 5}, WHITE, "Hello, world!");

    init_CUBES();

    for(i16 i = 0; true; i++) {
        // Check controls
        if(keyboard_is_key_down(KEY_ESC)) {
            break;
        }

        wait_for_vsync();
        clear_screen(0);

        graphics_text_print((struct Point){5, 5}, (i / 10) % 15 + 1, "Retrospect!");

        for (int i = 0; i < NUM_CUBES; i++) {
            CUBES[i].rotation.x += 1;
            CUBES[i].rotation.y += 1;
            CUBES[i].rotation.z += 1;
            CUBES[i].paletteIndex = CUBE_PALETTE_START + i * 3;
            shadeCube(&CUBES[i]);
            drawCube(&CUBES[i]);
        }
    }

    unhook_keyboard_interrupt();
    return 0;
}
