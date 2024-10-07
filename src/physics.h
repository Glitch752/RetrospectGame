#ifndef PHYSICS_H
#define PHYSICS_H

#define GRAVITY_FIXED_POINT TO_FIXED_POINT(-1)
#define GROUND_PLANE_HEIGHT TO_FIXED_POINT(-10)

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

void rotateVectorByCubeRotation(Vec3* vector, Cube* cube) {
    Vec3 rotatedVector = *vector;
    rotate_x(&rotatedVector, cube->rotation.x);
    rotate_y(&rotatedVector, cube->rotation.y);
    rotate_z(&rotatedVector, cube->rotation.z);
    *vector = rotatedVector;
}

void getSATCandidateAxes(Cube* cubeA, Cube* cubeB, Vec3* candidateAxes, int* numAxes) {
    *numAxes = 0;
    
    Vec3 normalsA[3] = {
        { TO_FIXED_POINT(1), TO_FIXED_POINT(0), TO_FIXED_POINT(0) },  // X-axis
        { TO_FIXED_POINT(0), TO_FIXED_POINT(1), TO_FIXED_POINT(0) },  // Y-axis
        { TO_FIXED_POINT(0), TO_FIXED_POINT(0), TO_FIXED_POINT(1) }   // Z-axis
    };

    Vec3 normalsB[3] = {
        { TO_FIXED_POINT(1), TO_FIXED_POINT(0), TO_FIXED_POINT(0) },  // X-axis
        { TO_FIXED_POINT(0), TO_FIXED_POINT(1), TO_FIXED_POINT(0) },  // Y-axis
        { TO_FIXED_POINT(0), TO_FIXED_POINT(0), TO_FIXED_POINT(1) }   // Z-axis
    };

    // Rotate face normals of both cubes
    for (int i = 0; i < 3; ++i) {
        rotateVectorByCubeRotation(&normalsA[i], cubeA);
        rotateVectorByCubeRotation(&normalsB[i], cubeB);
    }

    // Add normals from both cubes as candidate axes
    for (int i = 0; i < 3; ++i) {
        candidateAxes[(*numAxes)++] = normalsA[i];
        candidateAxes[(*numAxes)++] = normalsB[i];
    }

    // Cross products of edges of A and B as candidate axes
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vec3 crossProduct;
            crossProduct.x = FIXED_MUL(normalsA[i].y, normalsB[j].z) - FIXED_MUL(normalsA[i].z, normalsB[j].y);
            crossProduct.y = FIXED_MUL(normalsA[i].z, normalsB[j].x) - FIXED_MUL(normalsA[i].x, normalsB[j].z);
            crossProduct.z = FIXED_MUL(normalsA[i].x, normalsB[j].y) - FIXED_MUL(normalsA[i].y, normalsB[j].x);

            // Add cross product as an axis if non-zero
            if (crossProduct.x != 0 || crossProduct.y != 0 || crossProduct.z != 0) {
                candidateAxes[(*numAxes)++] = normalizeVectorFP(crossProduct);
            }
        }
    }
}

bool areCubesColliding(const Cube* cubeA, const Cube* cubeB) {
    Vec3 verticesA[8], verticesB[8];
    getCubeCorners(cubeA, verticesA);
    getCubeCorners(cubeB, verticesB);
    
    Vec3 axesToTest[15];
    int numAxes = 0;
    getSATCandidateAxes(cubeA, cubeB, axesToTest, &numAxes);

    // For each axis, project both cubes' vertices onto the axis and check for overlap
    for (int i = 0; i < numAxes; ++i) {
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

    // Update angular velocity using angular acceleration
    cube->angularVelocity.x += FIXED_MUL(cube->angularAccel.x, deltaTime);
    cube->angularVelocity.y += FIXED_MUL(cube->angularAccel.y, deltaTime);
    cube->angularVelocity.z += FIXED_MUL(cube->angularAccel.z, deltaTime);

    // Update rotation using angular velocity
    cube->rotation.x += FIXED_MUL(cube->angularVelocity.x, deltaTime);
    cube->rotation.y += FIXED_MUL(cube->angularVelocity.y, deltaTime);
    cube->rotation.z += FIXED_MUL(cube->angularVelocity.z, deltaTime);

    // Reset angular acceleration
    cube->angularAccel = (Vec3){0, 0, 0};
}

void applyForce(Cube* cube, Vec3 force) {
    cube->acceleration.x += FIXED_DIV(force.x, cube->mass);
    cube->acceleration.y += FIXED_DIV(force.y, cube->mass);
    cube->acceleration.z += FIXED_DIV(force.z, cube->mass);
}

void applyGravity(Cube* cube) {
    applyForce(cube, (Vec3){0, GRAVITY_FIXED_POINT, 0});
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

void applyCollisionAngularImpulse(Cube* cube, Vec3 collisionPoint, Vec3 collisionNormal) {
    // Calculate the lever arm: the vector from the cube's center to the collision point
    Vec3 r = vectorSubtract(&collisionPoint, &cube->position);

    // Compute torque: cross product of r and the impulse vector
    Vec3 torque = crossProductFP(&r, &collisionNormal);

    // Convert the impulse to angular acceleration: torque / moment of inertia
    i16 momentOfInertia = FIXED_DIV(
        FIXED_MUL(cube->mass, TO_FIXED_POINT(cube->size * cube->size)),
        TO_FIXED_POINT(6)
    );

    cube->angularVelocity.x += FIXED_DIV(torque.x, momentOfInertia);
    cube->angularVelocity.y += FIXED_DIV(torque.y, momentOfInertia);
    cube->angularVelocity.z += FIXED_DIV(torque.z, momentOfInertia);
}

i16 computePenetration(i16 minA, i16 maxA, i16 minB, i16 maxB) {
    // If there is no overlap, return 0 (no penetration)
    if (maxA < minB || maxB < minA) {
        return 0;
    }
    
    // Otherwise, return the depth of penetration
    return min(maxA - minB, maxB - minA);
}

Vec3 computeCollisionNormal(const Vec3* axes, int numAxes, Vec3* verticesA, Vec3* verticesB, i16 *penetrationDepth) {
    i16 minPenetration = 32767;
    Vec3 collisionNormal = {0, 0, 0};

    for (int i = 0; i < numAxes; ++i) {
        Vec3 axis = axes[i];

        // Project both cubes onto the axis
        i16 minA, maxA, minB, maxB;
        projectVerticesOnAxis(verticesA, axis, &minA, &maxA);
        projectVerticesOnAxis(verticesB, axis, &minB, &maxB);

        // Compute overlap on this axis
        i16 penetration = computePenetration(minA, maxA, minB, maxB);

        if (penetration < minPenetration) {
            minPenetration = penetration;
            collisionNormal = axis;
        }
    }

    *penetrationDepth = minPenetration;
    return collisionNormal;
}

Vec3 computeCollisionPoint(Vec3* verticesA, Vec3* verticesB, Vec3 collisionNormal) {
    Vec3 closestPointA = {0, 0, 0};
    Vec3 closestPointB = {0, 0, 0};
    i16 minDistance = INT16_MAX;

    // For each pair of vertices, calculate the distance between them along the collisionNormal
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            Vec3 pointA = verticesA[i];
            Vec3 pointB = verticesB[j];
            Vec3 difference = vectorSubtract(&pointA, &pointB);

            i16 distance = FIXED_MUL(difference.x, collisionNormal.x) +
                           FIXED_MUL(difference.y, collisionNormal.y) +
                           FIXED_MUL(difference.z, collisionNormal.z);

            if (abs(distance) < minDistance) {
                minDistance = abs(distance);
                closestPointA = pointA;
                closestPointB = pointB;
            }
        }
    }

    // Return the average of the closest points as the collision point
    Vec3 collisionPoint = vectorAdd(&closestPointA, &closestPointB);
    collisionPoint = vectorFPScale(&collisionPoint, FIXED_DIV(TO_FIXED_POINT(1), TO_FIXED_POINT(2)));
    return collisionPoint;
}

void resolveCollision(Cube* cubeA, Cube* cubeB, Vec3* verticesA, Vec3* verticesB, i16 restitution) {
    Vec3 axesToTest[15];
    int numAxes = 0;
    getSATCandidateAxes(cubeA, cubeB, axesToTest, &numAxes);
    
    i16 penetrationDepth = 0;
    Vec3 collisionNormal = computeCollisionNormal(axesToTest, numAxes, verticesA, verticesB, &penetrationDepth);

    Vec3 collisionPoint = computeCollisionPoint(verticesA, verticesB, collisionNormal);

    applyCollisionImpulse(cubeA, cubeB, collisionNormal);
    applyCollisionAngularImpulse(cubeA, collisionPoint, collisionNormal);
    applyCollisionAngularImpulse(cubeB, collisionPoint, collisionNormal);

    resolvePenetration(cubeA, cubeB, collisionNormal, penetrationDepth);
}

void simulate(i16 deltaTime) {
    for (int i = 0; i < NUM_CUBES; ++i) {
        // applyGravity(&CUBES[i]);
        updateCubePhysics(&CUBES[i], deltaTime);
    }

    // Collision detection and resolution
    for (int i = 0; i < NUM_CUBES; ++i) {
        for (int j = i + 1; j < NUM_CUBES; ++j) {
            if (areCubesColliding(&CUBES[i], &CUBES[j])) {
                Vec3 verticesA[8], verticesB[8];
                getCubeCorners(&CUBES[i], verticesA);
                getCubeCorners(&CUBES[j], verticesB);
                resolveCollision(&CUBES[i], &CUBES[j], verticesA, verticesB, TO_FIXED_POINT(5) >> 1);
            }
        }
    }
}

#endif