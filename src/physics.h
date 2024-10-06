#ifndef PHYSICS_H
#define PHYSICS_H

#define GRAVITY_FIXED_POINT TO_FIXED_POINT(-98) >> 1

bool intervalsOverlap(i16 minA, i16 maxA, i16 minB, i16 maxB) {
    return minA <= maxB && minB <= maxA;
}

void projectVerticesOnAxis(Vec3 vertices[], Vec3 axis, i16* min, i16* max) {
    *min = 32767;
    *max = -32768;

    for (int i = 0; i < 8; ++i) {
        i16 vertex = vertices[i].x * axis.x + vertices[i].y * axis.y + vertices[i].z * axis.z;
        if (vertex < *min) {
            *min = vertex;
        }
        if (vertex > *max) {
            *max = vertex;
        }
    }
}

bool areCubesColliding(const Cube* cubeA, const Cube* cubeB) {
    Vec3 verticesA[8], verticesB[8];
    getCubeCorners(cubeA, verticesA);
    getCubeCorners(cubeB, verticesB);
    
    // Define the 15 separating axes to test
    Vec3 axesToTest[15];
    int axisIndex = 0;

    // Normals of cubeA's faces (unit vectors along the x, y, and z axes)
    axesToTest[axisIndex++] = (Vec3){1, 0, 0};  // X-axis
    axesToTest[axisIndex++] = (Vec3){0, 1, 0};  // Y-axis
    axesToTest[axisIndex++] = (Vec3){0, 0, 1};  // Z-axis

    // Normals of cubeB's faces (if rotated, need to compute them)
    // Assume cubeB is also axis-aligned, for simplicity
    axesToTest[axisIndex++] = (Vec3){1, 0, 0};
    axesToTest[axisIndex++] = (Vec3){0, 1, 0};
    axesToTest[axisIndex++] = (Vec3){0, 0, 1};

    // Cross products of edges (for rotating cubes)
    Vec3 edgesA[3] = {(Vec3){1, 0, 0}, (Vec3){0, 1, 0}, (Vec3){0, 0, 1}};
    Vec3 edgesB[3] = {(Vec3){1, 0, 0}, (Vec3){0, 1, 0}, (Vec3){0, 0, 1}};

    // Cross product of edges between both cubes
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            axesToTest[axisIndex++] = crossProduct(&edgesA[i], &edgesB[j]);
        }
    }

    // For each axis, project both cubes' vertices onto the axis and check for overlap
    for (int i = 0; i < axisIndex; ++i) {
        Vec3 axis = axesToTest[i];
        i16 minA, maxA, minB, maxB;

        projectVerticesOnAxis(verticesA, axis, &minA, &maxA);
        projectVerticesOnAxis(verticesB, axis, &minB, &maxB);

        if (!intervalsOverlap(minA, maxA, minB, maxB)) {
            // If there is any axis where projections do not overlap, no collision
            return false;
        }
    }

    // If all projections overlap, there is a collision
    return true;
}

void updateCubePhysics(Cube* cube, i16 deltaTime) {
    // Update velocity using acceleration (fixed-point multiplication)
    cube->velocity.x += FIXED_MUL(cube->acceleration.x, deltaTime);
    cube->velocity.y += FIXED_MUL(cube->acceleration.y, deltaTime);
    cube->velocity.z += FIXED_MUL(cube->acceleration.z, deltaTime);

    // Update position using velocity (fixed-point multiplication)
    cube->position.x += FIXED_MUL(cube->velocity.x, deltaTime);
    cube->position.y += FIXED_MUL(cube->velocity.y, deltaTime);
    cube->position.z += FIXED_MUL(cube->velocity.z, deltaTime);

    // Reset acceleration
    cube->acceleration = (Vec3){0, 0, 0};
}

void applyForce(Cube* cube, Vec3 force) {
    cube->acceleration.x += FIXED_DIV(force.x, cube->mass);
    cube->acceleration.y += FIXED_DIV(force.y, cube->mass);
    cube->acceleration.z += FIXED_DIV(force.z, cube->mass);
}

void applyGravity(Cube* cube) {
    Vec3 gravity = {0, GRAVITY_FIXED_POINT, 0}; // Gravity constant in fixed-point (Q8.8)
    applyForce(cube, gravity);
}

void resolvePenetration(Cube* cubeA, Cube* cubeB, Vec3 collisionNormal, i16 penetrationDepth) {
    // Push cubes apart proportionally to their mass (all values in fixed-point)
    i16 totalMass = cubeA->mass + cubeB->mass;

    Vec3 correction = vectorFPScale(&collisionNormal, FIXED_DIV(penetrationDepth, totalMass));

    cubeA->position = vectorSubtract(&cubeA->position, &correction);
    cubeB->position = vectorAdd(&cubeB->position, &correction);
}

void applyCollisionImpulse(Cube* cubeA, Cube* cubeB, Vec3 collisionNormal) {
    Vec3 relativeVelocity = {
        cubeB->velocity.x - cubeA->velocity.x,
        cubeB->velocity.y - cubeA->velocity.y,
        cubeB->velocity.z - cubeA->velocity.z
    };

    i16 velocityAlongNormal = FIXED_MUL(relativeVelocity.x, collisionNormal.x) +
                              FIXED_MUL(relativeVelocity.y, collisionNormal.y) +
                              FIXED_MUL(relativeVelocity.z, collisionNormal.z);

    // Only resolve if cubes are moving toward each other
    if (velocityAlongNormal > 0) return;

    // Coefficient of restitution (elasticity of the collision)
    i16 restitution = TO_FIXED_POINT(5) >> 1; // 0 = inelastic, 1 = elastic

    // Impulse magnitude
    i16 impulseMagnitude = FIXED_MUL(-(1 + restitution), velocityAlongNormal);
    impulseMagnitude = FIXED_DIV(impulseMagnitude, (FIXED_DIV(TO_FIXED_POINT(1), cubeA->mass) + FIXED_DIV(TO_FIXED_POINT(1), cubeB->mass)));

    // Apply impulse
    Vec3 impulse = vectorFPScale(&collisionNormal, impulseMagnitude);

    cubeA->velocity.x -= FIXED_DIV(impulse.x, cubeA->mass);
    cubeA->velocity.y -= FIXED_DIV(impulse.y, cubeA->mass);
    cubeA->velocity.z -= FIXED_DIV(impulse.z, cubeA->mass);

    cubeB->velocity.x += FIXED_DIV(impulse.x, cubeB->mass);
    cubeB->velocity.y += FIXED_DIV(impulse.y, cubeB->mass);
    cubeB->velocity.z += FIXED_DIV(impulse.z, cubeB->mass);
}

void simulate(i16 deltaTime) {
    for (int i = 0; i < NUM_CUBES; ++i) {
        applyGravity(&CUBES[i]);
        updateCubePhysics(&CUBES[i], deltaTime);
    }

    // Collision detection and resolution
    for (int i = 0; i < NUM_CUBES; ++i) {
        for (int j = i + 1; j < NUM_CUBES; ++j) {
            if (areCubesColliding(&CUBES[i], &CUBES[j])) {
                Vec3 collisionNormal;  // Calculate this from the collision
                i16 penetrationDepth;  // Calculate this from the collision

                // Resolve penetration and apply impulse
                resolvePenetration(&CUBES[i], &CUBES[j], collisionNormal, penetrationDepth);
                applyCollisionImpulse(&CUBES[i], &CUBES[j], collisionNormal);
            }
        }
    }
}

#endif