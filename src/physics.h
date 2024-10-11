#ifndef PHYSICS_H
#define PHYSICS_H

#include "types.h"
#include "vec3.h"
#include "objects.h"


void simulate(i16 deltaTime) {
    for(int a = 0; a < NUM_CUBES; a++) {
        for(int b = a + 1; b < NUM_CUBES; b++) {
            // TODO: detect and resolve collisions
        }
    }
}

#endif