import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';

// We're intentionally not using a lot of classes and three.js concepts here to make it easy to port to C.
// This is bad code, I know. I'm sorry.

const scene = new THREE.Scene();

const gravity = 0.00049;

function quaternionMultiply(w1: number, i1: number, j1: number, k1: number, w2: number, i2: number, j2: number, k2: number) {
  return [
    w1 * w2 - i1 * i2 - j1 * j2 - k1 * k2,
    w1 * i2 + i1 * w2 + j1 * k2 - k1 * j2,
    w1 * j2 - i1 * k2 + j1 * w2 + k1 * i2,
    w1 * k2 + i1 * j2 - j1 * i2 + k1 * w2
  ];
}

class Cube {
  x: number = 0;
  y: number = 0;
  z: number = 0;

  size: number = 0;

  rotationX: number = 0;
  rotationY: number = 0;
  rotationZ: number = 0;
  
  velocityX: number = 0;
  velocityY: number = 0;
  velocityZ: number = 0;

  recentMotion: number = 0;
  recentRotation: number = 0;

  sleeping: boolean = false;

  t00: number = 1; t01: number = 0; t02: number = 0;
  t10: number = 0; t11: number = 1; t12: number = 0;
  t20: number = 0; t21: number = 0; t22: number = 1;

  w: number = 1; i: number = 0; j: number = 0; k: number = 0;

  maximumPenetration: number = 0;
  maximumPenetrationCollision: number = 0;

  minX: number = 0; minY: number = 0; minZ: number = 0;
  minXVertex: number = 0; minYVertex: number = 0; minZVertex: number = 0;
  maxX: number = 0; maxY: number = 0; maxZ: number = 0;
  maxXVertex: number = 0; maxYVertex: number = 0; maxZVertex: number = 0;

  vertices: [number, number, number][] = [[0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]];
  axes: [number, number, number][] = [[0, 0, 0], [0, 0, 0], [0, 0, 0]];

  constructor(size: number, x: number, y: number, z: number) {
    this.size = size;
    this.x = x;
    this.y = y;
    this.z = z;
  }

  physicsTick() {
    // TEMPORARY
    // this.velocityY -= gravity;
    
    this.integrate();
    this.quaternionToTransform();
  }

  quaternionToTransform() {
    this.t00 = 1 - 2 * this.j * this.j - 2 * this.k * this.k;
    this.t01 = 2 * this.i * this.j - 2 * this.k * this.w;
    this.t02 = 2 * this.i * this.k + 2 * this.j * this.w;

    this.t10 = 2 * this.i * this.j + 2 * this.k * this.w;
    this.t11 = 1 - 2 * this.i * this.i - 2 * this.k * this.k;
    this.t12 = 2 * this.j * this.k - 2 * this.i * this.w;

    this.t20 = 2 * this.i * this.k - 2 * this.j * this.w;
    this.t21 = 2 * this.j * this.k + 2 * this.i * this.w;
    this.t22 = 1 - 2 * this.i * this.i - 2 * this.j * this.j;
  }

  checkSleep() {
    if(this.recentMotion < 0.0000001 && this.recentRotation < 0.0000001) {
      this.sleep();
    }
  }

  sleep() {
    this.velocityX = 0;
    this.velocityY = 0;
    this.velocityZ = 0;

    this.rotationX = 0;
    this.rotationY = 0;
    this.rotationZ = 0;

    this.sleeping = true;
  }
  wakeUp() {
    this.recentMotion = 10;
    this.recentRotation = 10;
    this.sleeping = false;
  }

  integrate() {
    this.integrateVelocity();
    this.integrateRotation();
  }

  integrateVelocity() {
    // WHEN CHANGING TO FIXED POINT, add half the precision to round correctly
    this.velocityX = this.velocityX * 0.995;
    this.velocityY = this.velocityY * 0.995;
    this.velocityZ = this.velocityZ * 0.995;

    this.x += this.velocityX;
    this.y += this.velocityY;
    this.z += this.velocityZ;

    const motion = this.velocityX * this.velocityX + this.velocityY * this.velocityY + this.velocityZ * this.velocityZ;
    this.recentMotion = this.recentMotion * 0.9 + motion;
  }

  integrateRotation() {
    // WHEN CHANGING TO FIXED POINT, add half the precision to round correctly
    this.rotationX = this.rotationX * 0.995;
    this.rotationY = this.rotationY * 0.995;
    this.rotationZ = this.rotationZ * 0.995;

    const result = quaternionMultiply(0, this.rotationX, this.rotationY, this.rotationZ, this.w, this.i, this.j, this.k);
    this.w += result[0] / 2; this.i += result[1] / 2; this.j += result[2] / 2; this.k += result[3] / 2;

    this.normalizeQuaternion();

    const rotation = this.rotationX * this.rotationX + this.rotationY * this.rotationY + this.rotationZ * this.rotationZ;
    this.recentRotation = this.recentRotation * 0.9 + rotation;
  }

  normalizeQuaternion() {
    const mag = Math.sqrt(this.w*this.w + this.i*this.i + this.j*this.j + this.k*this.k);
    // WHEN CHANGING TO FIXED POINT, add half the precision to round correctly
    this.w /= mag; this.i /= mag; this.j /= mag; this.k /= mag;
  }

  calculateVertexWorldPositions() {
    const halfSize = this.size / 2;
    
    // Calculate the AABB
    this.minX = this.x; this.minY = this.y; this.minZ = this.z;
    this.maxX = this.x; this.maxY = this.y; this.maxZ = this.z;

    // Initialize all vertices to our position
    for(let i = 0; i < 8; i++) {
      this.vertices[i][0] = this.x;
      this.vertices[i][1] = this.y;
      this.vertices[i][2] = this.z;
    }

    this.axes[0] = this.localToWorld(1, 0, 0, false);
    let [worldX, worldY, worldZ] = this.axes[0];
    worldX *= halfSize; worldY *= halfSize; worldZ *= halfSize;

    this.vertices[0][0] += worldX; this.vertices[0][1] += worldY; this.vertices[0][2] += worldZ;
    this.vertices[1][0] -= worldX; this.vertices[1][1] -= worldY; this.vertices[1][2] -= worldZ;
    this.vertices[2][0] += worldX; this.vertices[2][1] += worldY; this.vertices[2][2] += worldZ;
    this.vertices[3][0] -= worldX; this.vertices[3][1] -= worldY; this.vertices[3][2] -= worldZ;
    this.vertices[4][0] += worldX; this.vertices[4][1] += worldY; this.vertices[4][2] += worldZ;
    this.vertices[5][0] -= worldX; this.vertices[5][1] -= worldY; this.vertices[5][2] -= worldZ;
    this.vertices[6][0] += worldX; this.vertices[6][1] += worldY; this.vertices[6][2] += worldZ;
    this.vertices[7][0] -= worldX; this.vertices[7][1] -= worldY; this.vertices[7][2] -= worldZ;

    this.axes[1] = this.localToWorld(0, 1, 0, false);
    [worldX, worldY, worldZ] = this.axes[1];
    worldX *= halfSize; worldY *= halfSize; worldZ *= halfSize;

    this.vertices[0][0] += worldX; this.vertices[0][1] += worldY; this.vertices[0][2] += worldZ;
    this.vertices[1][0] -= worldX; this.vertices[1][1] -= worldY; this.vertices[1][2] -= worldZ;
    this.vertices[2][0] -= worldX; this.vertices[2][1] -= worldY; this.vertices[2][2] -= worldZ;
    this.vertices[3][0] += worldX; this.vertices[3][1] += worldY; this.vertices[3][2] += worldZ;
    this.vertices[4][0] += worldX; this.vertices[4][1] += worldY; this.vertices[4][2] += worldZ;
    this.vertices[5][0] -= worldX; this.vertices[5][1] -= worldY; this.vertices[5][2] -= worldZ;
    this.vertices[6][0] -= worldX; this.vertices[6][1] -= worldY; this.vertices[6][2] -= worldZ;
    this.vertices[7][0] += worldX; this.vertices[7][1] += worldY; this.vertices[7][2] += worldZ;

    this.axes[2] = this.localToWorld(0, 0, 1, false);
    [worldX, worldY, worldZ] = this.axes[2];
    worldX *= halfSize; worldY *= halfSize; worldZ *= halfSize;

    this.vertices[0][0] += worldX; this.vertices[0][1] += worldY; this.vertices[0][2] += worldZ;
    this.vertices[1][0] -= worldX; this.vertices[1][1] -= worldY; this.vertices[1][2] -= worldZ;
    this.vertices[2][0] += worldX; this.vertices[2][1] += worldY; this.vertices[2][2] += worldZ;
    this.vertices[3][0] -= worldX; this.vertices[3][1] -= worldY; this.vertices[3][2] -= worldZ;
    this.vertices[4][0] -= worldX; this.vertices[4][1] -= worldY; this.vertices[4][2] -= worldZ;
    this.vertices[5][0] += worldX; this.vertices[5][1] += worldY; this.vertices[5][2] += worldZ;
    this.vertices[6][0] -= worldX; this.vertices[6][1] -= worldY; this.vertices[6][2] -= worldZ;
    this.vertices[7][0] += worldX; this.vertices[7][1] += worldY; this.vertices[7][2] += worldZ;

    // Update the AABB based on the vertices
    for(let v = 0; v < 8; v++) {
      const vertex = this.vertices[v];
      if(vertex[0] < this.minX) { this.minX = vertex[0]; this.minXVertex = v; }
      if(vertex[0] > this.maxX) { this.maxX = vertex[0]; this.maxXVertex = v; }
      if(vertex[1] < this.minY) { this.minY = vertex[1]; this.minYVertex = v; }
      if(vertex[1] > this.maxY) { this.maxY = vertex[1]; this.maxYVertex = v; }
      if(vertex[2] < this.minZ) { this.minZ = vertex[2]; this.minZVertex = v; }
      if(vertex[2] > this.maxZ) { this.maxZ = vertex[2]; this.maxZVertex = v; }
    }
  }

  localToWorld(x: number, y: number, z: number, includeTranslation: boolean): [number, number, number] {
    let worldX = this.t00 * x + this.t01 * y + this.t02 * z;
    let worldY = this.t10 * x + this.t11 * y + this.t12 * z;
    let worldZ = this.t20 * x + this.t21 * y + this.t22 * z;
    
    if(includeTranslation) {
      worldX += this.x;
      worldY += this.y;
      worldZ += this.z;
    }

    return [worldX, worldY, worldZ];
  }

  worldToLocal(x: number, y: number, z: number, includeTranslation: boolean): [number, number, number] {
    if(includeTranslation) {
      x -= this.x;
      y -= this.y;
      z -= this.z;
    }
    
    return [
      this.t00 * x + this.t10 * y + this.t20 * z,
      this.t01 * x + this.t11 * y + this.t21 * z,
      this.t02 * x + this.t12 * y + this.t22 * z
    ];
  }

  mesh?: THREE.Mesh;
  tempMatrix3 = new THREE.Matrix3();
  tempVector3 = new THREE.Vector3();
  updateRendering() {
    if(!this.mesh) return;
    this.mesh.matrixAutoUpdate = false;
    this.mesh.matrix.setFromMatrix3(this.tempMatrix3.set(
      this.t00 * this.size, this.t01 * this.size, this.t02 * this.size,
      this.t10 * this.size, this.t11 * this.size, this.t12 * this.size,
      this.t20 * this.size, this.t21 * this.size, this.t22 * this.size
    ));
    this.mesh.matrix.setPosition(this.tempVector3.set(this.x, this.y, this.z));
  }
}

/* Returns the two closest points on the edges of the two colliders. Returns null if either point is outside its edge. */
function getTwoClosestPointsOnEdges(
  collider1Vertex: [number, number, number], collider1Edge: [number, number, number],
  collider2Vertex: [number, number, number], collider2Edge: [number, number, number]
): [number, number, number, number, number, number] | null {
  const normalX = collider1Edge[1] * collider2Edge[2] - collider1Edge[2] * collider2Edge[1];
  const normalY = collider1Edge[2] * collider2Edge[0] - collider1Edge[0] * collider2Edge[2];
  const normalZ = collider1Edge[0] * collider2Edge[1] - collider1Edge[1] * collider2Edge[0];

  const squaredNormalMagnitude = normalX*normalX + normalY*normalY + normalZ*normalZ;

  const originDifferenceX = collider2Vertex[0] - collider1Vertex[0];
  const originDifferenceY = collider2Vertex[1] - collider1Vertex[1];
  const originDifferenceZ = collider2Vertex[2] - collider1Vertex[2];

  const perpendicularX = collider2Edge[1] * normalZ - collider2Edge[2] * normalY;
  const perpendicularY = collider2Edge[2] * normalX - collider2Edge[0] * normalZ;
  const perpendicularZ = collider2Edge[0] * normalY - collider2Edge[1] * normalX;
  const collider1Position = (perpendicularX * originDifferenceX + perpendicularY * originDifferenceY + perpendicularZ * originDifferenceZ) / squaredNormalMagnitude;
  if(collider1Position < 0 || collider1Position > 1) return null;

  const perpendicularX2 = (collider1Edge[1] * normalZ - collider1Edge[2] * normalY);
  const perpendicularY2 = (collider1Edge[2] * normalX - collider1Edge[0] * normalZ);
  const perpendicularZ2 = (collider1Edge[0] * normalY - collider1Edge[1] * normalX);
  const collider2Position = (perpendicularX2 * originDifferenceX + perpendicularY2 * originDifferenceY + perpendicularZ2 * originDifferenceZ) / squaredNormalMagnitude;
  if(collider1Position < 0 || collider1Position > 1) return null;

  const point1X = collider1Vertex[0] + collider1Position * collider1Edge[0];
  const point1Y = collider1Vertex[1] + collider1Position * collider1Edge[1];
  const point1Z = collider1Vertex[2] + collider1Position * collider1Edge[2];

  const point2X = collider2Vertex[0] + collider2Position * collider2Edge[0];
  const point2Y = collider2Vertex[1] + collider2Position * collider2Edge[1];
  const point2Z = collider2Vertex[2] + collider2Position * collider2Edge[2];

  return [point1X, point1Y, point1Z, point2X, point2Y, point2Z];
}

enum CollisionType {
  CubeFaceCubePoint,
  CubeEdgeCubeEdge
}

class Collision {
  active: boolean = false;
  type: CollisionType = 0;
  
  // For CubeFaceCubePoint
  vertex: number = 0;
  faceX: number = 0; faceY: number = 0; faceZ: number = 0;
  
  // For CubeEdgeCubeEdge
  vertex1: number = 0; nextVertex1: number = 0;
  vertex2: number = 0; nextVertex2: number = 0;

  penetration: number = 0;
  collider1: Cube; collider2: Cube;

  // Results
  vertexX: number = 0; vertexY: number = 0; vertexZ: number = 0;
  worldX: number = 0; worldY: number = 0; worldZ: number = 0;
  directionX: number = 0; directionY: number = 0; directionZ: number = 0;

  private constructor(type: CollisionType, collider1: Cube, collider2: Cube) {
    this.type = type;
    this.collider1 = collider1;
    this.collider2 = collider2;
  }

  static cubeFaceCubePoint(collider1: Cube, collider2: Cube, vertex: number, faceX: number, faceY: number, faceZ: number, vertexPosition: [number, number, number], directionX: number, directionY: number, directionZ: number) {
    const collision = new Collision(CollisionType.CubeFaceCubePoint, collider1, collider2);
    collision.faceX = faceX;
    collision.faceY = faceY;
    collision.faceZ = faceZ;
    collision.vertex = vertex;
    collision.vertexX = vertexPosition[0];
    collision.vertexY = vertexPosition[1];
    collision.vertexZ = vertexPosition[2];
    collision.directionX = directionX;
    collision.directionY = directionY;
    collision.directionZ = directionZ;
    return collision;
  }

  static cubeEdgeCubeEdge(collider1: Cube, collider2: Cube, vertex1: number, vertex2: number, nextVertex1: number, nextVertex2: number, directionX: number, directionY: number, directionZ: number, worldX: number, worldY: number, worldZ: number) {
    const collision = new Collision(CollisionType.CubeEdgeCubeEdge, collider1, collider2);
    collision.vertex1 = vertex1;
    collision.vertex2 = vertex2;
    collision.nextVertex1 = nextVertex1;
    collision.nextVertex2 = nextVertex2;
    collision.directionX = directionX;
    collision.directionY = directionY;
    collision.directionZ = directionZ;
    collision.worldX = worldX;
    collision.worldY = worldY;
    collision.worldZ = worldZ;
    return collision;
  }

  /* Returns if the collision should be removed */
  update(): boolean {
    switch(this.type) {
      case CollisionType.CubeFaceCubePoint: {
        const vertexPosition = this.collider2.vertices[this.vertex];
        
        let [worldX, worldY, worldZ] = this.collider1.localToWorld(this.faceX, this.faceY, this.faceZ, false);
        this.worldX = worldX; this.worldY = worldY; this.worldZ = worldZ;
        
        let [localX, localY, localZ] = this.collider1.worldToLocal(vertexPosition[0], vertexPosition[1], vertexPosition[2], true);
    
        const halfSize = this.collider1.size / 2;

        let keep = false;
        if(this.faceX == -1 && localY >= -halfSize && localY <= halfSize && localZ >= -halfSize && localZ <= halfSize) {
          this.penetration = halfSize - localX;
          keep = true;
        }
        if(this.faceX == 1 && localY >= -halfSize && localY <= halfSize && localZ >= -halfSize && localZ <= halfSize) {
          this.penetration = localX + halfSize;
          keep = true;
        }
        if(this.faceY == -1 && localX >= -halfSize && localX <= halfSize && localZ >= -halfSize && localZ <= halfSize) {
          this.penetration = halfSize - localY;
          keep = true;
        }
        if(this.faceY == 1 && localX >= -halfSize && localX <= halfSize && localZ >= -halfSize && localZ <= halfSize) {
          this.penetration = localY + halfSize;
          keep = true;
        }
        if(this.faceZ == -1 && localX >= -halfSize && localX <= halfSize && localY >= -halfSize && localY <= halfSize) {
          this.penetration = halfSize - localZ;
          keep = true;
        }
        if(this.faceZ == 1 && localX >= -halfSize && localX <= halfSize && localY >= -halfSize && localY <= halfSize) {
          this.penetration = localZ + halfSize;
          keep = true;
        }
        
        if(keep) {
          this.vertexX = vertexPosition[0]; this.vertexY = vertexPosition[1]; this.vertexZ = vertexPosition[2];
        } else {
          return true; // Remove the collision
        }
        break;
      }
      case CollisionType.CubeEdgeCubeEdge: {
        const collider1Vertex = this.collider1.vertices[this.vertex1];
        const collider1Edge = [
          this.collider1.vertices[this.nextVertex1][0] - collider1Vertex[0],
          this.collider1.vertices[this.nextVertex1][1] - collider1Vertex[1],
          this.collider1.vertices[this.nextVertex1][2] - collider1Vertex[2]
        ] as [number, number, number];

        const collider2Vertex = this.collider2.vertices[this.vertex2];
        const collider2Edge = [
          this.collider2.vertices[this.nextVertex2][0] - collider2Vertex[0],
          this.collider2.vertices[this.nextVertex2][1] - collider2Vertex[1],
          this.collider2.vertices[this.nextVertex2][2] - collider2Vertex[2]
        ] as [number, number, number];
        
        const points = getTwoClosestPointsOnEdges(collider1Vertex, collider1Edge, collider2Vertex, collider2Edge);
        if(points) {
          const [p1x, p1y, p1z, p2x, p2y, p2z] = points;
          
          const [localX, localY, localZ] = this.collider2.worldToLocal(p1x, p1y, p1z, true);
          const collider2HalfSize = this.collider2.size / 2;

          if(localX >= -collider2HalfSize && localX >= collider2HalfSize && localY >= -collider2HalfSize && localY <= collider2HalfSize && localZ >= -collider2HalfSize && localZ <= collider2HalfSize) {
            // Collision point falls within collider2
            const [localX, localY, localZ] = this.collider1.worldToLocal(p2x, p2y, p2z, true);
            const collider1HalfSize = this.collider1.size / 2;

            if(localX >= -collider1HalfSize && localX >= collider1HalfSize && localY >= -collider1HalfSize && localY <= collider1HalfSize && localZ >= -collider1HalfSize && localZ <= collider1HalfSize) {
              // Collision point falls within collider1
              const dx = p2x - p1x, dy = p2y - p1y, dz = p2z - p1z;
              const penetration = Math.sqrt(dx*dx + dy*dy + dz*dz);
              this.penetration = penetration;
              this.vertexX = p1x; this.vertexY = p1y; this.vertexZ = p1z;
              this.worldX = dx / penetration; this.worldY = dy / penetration; this.worldZ = dz / penetration;
            } else return true; // Remove the collision
          } else return true; // Remove the collision
        } else return true; // Remove the collision
        break;
      }
      default: return true; // Remove the collision
    }

    if(this.penetration < -0.1) return true; // Remove the collision
    if(this.penetration > this.collider1.maximumPenetration) {
      this.collider1.maximumPenetration = this.penetration;
    }
    if(this.penetration > this.collider2.maximumPenetration) {
      this.collider2.maximumPenetration = this.penetration;
    }

    return false;
  }
}

const cubes: Cube[] = [];
const collisions: Collision[] = [];
for(let x = -2; x <= 2; x++) for(let y = -2; y <= 2; y++) cubes.push(new Cube(0.1, x * 0.2, 1.5, y * 0.2));

function updateActiveCollisions() {
  // Update the maximum penetration for each collider
  for(let cube of cubes) {
    cube.calculateVertexWorldPositions();
    cube.maximumPenetration = 0;
    cube.maximumPenetrationCollision = 0;
  }

  // Check for collisions
  for(let i = 0; i < collisions.length; i++) {
    const collision = collisions[i];
    if(collision.active) {
      if(collision.update()) {
        // Remove the collision
        collisions.splice(i, 1);
        i--;
      }
    }
  }
}

function projectOntoSeparatingAxis(collider: Cube, separatingAxis: [number, number, number]): number {
  let projected = 0;
  for(let axisIndex = 0; axisIndex < 3; axisIndex++) {
    const axis = collider.axes[axisIndex];
    let dot = axis[0] * separatingAxis[0] + axis[1] * separatingAxis[1] + axis[2] * separatingAxis[2];
    if(dot < 0) dot *= -1;
    projected += dot;
  }

  return projected * collider.size / 2;
}

function testAxisPenetration(centerToCenter: [number, number, number], axis: [number, number, number], collider1: Cube, collider2: Cube):
  { axisSign: number, penetration: number } {
  // Initialize the separation to be the distance between centers along the axis
  let penetration = centerToCenter[0] * axis[0] + centerToCenter[1] * axis[1] + centerToCenter[2] * axis[2];
  let axisSign = 1;

  if(penetration > 0) {
    penetration *= -1;
    axisSign = -1;
  }

  penetration += projectOntoSeparatingAxis(collider1, axis);
  penetration += projectOntoSeparatingAxis(collider2, axis);

  return { axisSign, penetration };
}

function getMinimumSeparatingAxis(collider1: Cube, collider2: Cube): { axisIndex: number, axis: [number, number, number], axisSign: number, penetration: number } | null {
  const penetrationThreshold = collider1.maximumPenetration < collider2.maximumPenetration ? collider1.maximumPenetration : collider2.maximumPenetration;
  let minimumPenetration = Infinity;
  let minimumAxis: { axisIndex: number, axis: [number, number, number], axisSign: number, penetration: number } | null = null;
  
  const centerToCenter = [
    collider2.x - collider1.x,
    collider2.y - collider1.y,
    collider2.z - collider1.z
  ] as [number, number, number];

  const distanceSquared = centerToCenter[0]*centerToCenter[0] + centerToCenter[1]*centerToCenter[1] + centerToCenter[2]*centerToCenter[2];
  const halfSize = collider1.size / 2 + collider2.size / 2;
  const boundingSphereSquared = halfSize * halfSize * 1.8; // 1.8 may need to be adjusted
  
  if(distanceSquared > boundingSphereSquared) return null;
  
  let axisIndex = 0;
  for(let i = 0; i < 3; i++) {
    const axis = collider1.axes[i];
    const penetration = testAxisPenetration(centerToCenter, axis, collider1, collider2);
    axisIndex++;
    if(penetration.penetration <= penetrationThreshold) return { axisIndex, axis, ...penetration };
    if(penetration.penetration < minimumPenetration) {
      minimumPenetration = penetration.penetration;
      minimumAxis = { axisIndex, axis, ...penetration };
    }
  }

  for(let i = 0; i < 3; i++) {
    const axis = collider2.axes[i];
    const penetration = testAxisPenetration(centerToCenter, axis, collider1, collider2);
    axisIndex++;
    if(penetration.penetration <= penetrationThreshold) return { axisIndex, axis, ...penetration };
    if(penetration.penetration < minimumPenetration) {
      minimumPenetration = penetration.penetration;
      minimumAxis = { axisIndex, axis, ...penetration };
    }
  }

  for(let axisIndexA = 0; axisIndexA < 3; axisIndexA++) {
    const axis1 = collider1.axes[axisIndexA];
    for(let axisIndexB = 0; axisIndexB < 3; axisIndexB++) {
      const axis2 = collider2.axes[axisIndexB];
      axisIndex++;

      let separatingAxis = [
        axis1[1] * axis2[2] - axis1[2] * axis2[1],
        axis1[2] * axis2[0] - axis1[0] * axis2[2],
        axis1[0] * axis2[1] - axis1[1] * axis2[0]
      ] as [number, number, number];

      let magnitudeSquared = separatingAxis[0] * separatingAxis[0] + separatingAxis[1] * separatingAxis[1] + separatingAxis[2] * separatingAxis[2];
      if(magnitudeSquared < 0.0001) continue; // The axes are parallel

      const magnitude = Math.sqrt(magnitudeSquared);

      separatingAxis[0] /= magnitude; separatingAxis[1] /= magnitude; separatingAxis[2] /= magnitude;

      const penetration = testAxisPenetration(centerToCenter, separatingAxis, collider1, collider2);
      if(penetration.penetration <= penetrationThreshold) return { axisIndex, axis: separatingAxis, ...penetration };
      if(penetration.penetration < minimumPenetration) {
        minimumPenetration = penetration.penetration;
        minimumAxis = { axisIndex, axis: separatingAxis, ...penetration };
      }
    }
  }

  return minimumAxis;
}

function getVertexFromAxisSigns(sign0: number, sign1: number, sign2: number): number {
  // TODO
}

function getEdgeFromSigns(sign0: number, sign1: number, sign2: number): { vertex: number, nextVertex: number } {
  // TODO
}

function getCollisions(collider1: Cube, collider2: Cube) {
  if(collider1.sleeping && collider2.sleeping) return;

  const minimumSeparatingAxis = getMinimumSeparatingAxis(collider1, collider2);
  if(!minimumSeparatingAxis) return;

  if(minimumSeparatingAxis.penetration > collider1.maximumPenetration) {
    collider1.maximumPenetration = minimumSeparatingAxis.penetration;
    collider1.maximumPenetrationCollision = collisions.length;
  }

  if(minimumSeparatingAxis.penetration > collider2.maximumPenetration) {
    collider2.maximumPenetration = minimumSeparatingAxis.penetration;
    collider2.maximumPenetrationCollision = collisions.length;
  }

  if(minimumSeparatingAxis.axisIndex < 3) {
    // Collider1 face, collider2 point
    let directionX = 0, directionY = 0, directionZ = 0;
    if(minimumSeparatingAxis.axisIndex == 0) {
      directionX = minimumSeparatingAxis.axisSign; directionY = 0; directionZ = 0;
    } else if(minimumSeparatingAxis.axisIndex == 1) {
      directionX = 0; directionY = minimumSeparatingAxis.axisSign; directionZ = 0;
    } else if(minimumSeparatingAxis.axisIndex == 2) {
      directionX = 0; directionY = 0; directionZ = minimumSeparatingAxis.axisSign;
    }

    const axis0 = collider1.axes[0];
    const axis1 = collider1.axes[1];
    const axis2 = collider1.axes[2];

    let minAxisX = minimumSeparatingAxis.axis[0] * minimumSeparatingAxis.axisSign;
    let minAxisY = minimumSeparatingAxis.axis[1] * minimumSeparatingAxis.axisSign;
    let minAxisZ = minimumSeparatingAxis.axis[2] * minimumSeparatingAxis.axisSign;

    let sign0 = minAxisX * axis0[0] + minAxisY * axis0[1] + minAxisZ * axis0[2];
    let sign1 = minAxisX * axis1[0] + minAxisY * axis1[1] + minAxisZ * axis1[2];
    let sign2 = minAxisX * axis2[0] + minAxisY * axis2[1] + minAxisZ * axis2[2];

    let vertex = getVertexFromAxisSigns(sign0, sign1, sign2);

    const collision = Collision.cubeFaceCubePoint(collider1, collider2, vertex, minAxisX, minAxisY, minAxisZ, collider2.vertices[vertex], directionX, directionY, directionZ);
    collisions.push(collision);
  } else if(minimumSeparatingAxis.axisIndex < 6) {
    // Collider1 point, collider2 face
    let minAxisSign = minimumSeparatingAxis.axisSign * -1;
    let directionX = 0, directionY = 0, directionZ = 0;
    if(minimumSeparatingAxis.axisIndex == 3) {
      directionX = minimumSeparatingAxis.axisSign; directionY = 0; directionZ = 0;
    } else if(minimumSeparatingAxis.axisIndex == 4) {
      directionX = 0; directionY = minimumSeparatingAxis.axisSign; directionZ = 0;
    } else if(minimumSeparatingAxis.axisIndex == 5) {
      directionX = 0; directionY = 0; directionZ = minimumSeparatingAxis.axisSign;
    }

    const axis0 = collider2.axes[0];
    const axis1 = collider2.axes[1];
    const axis2 = collider2.axes[2];

    let minAxisX = minimumSeparatingAxis.axis[0] * minAxisSign;
    let minAxisY = minimumSeparatingAxis.axis[1] * minAxisSign;
    let minAxisZ = minimumSeparatingAxis.axis[2] * minAxisSign;

    let sign0 = minAxisX * axis0[0] + minAxisY * axis0[1] + minAxisZ * axis0[2];
    let sign1 = minAxisX * axis1[0] + minAxisY * axis1[1] + minAxisZ * axis1[2];
    let sign2 = minAxisX * axis2[0] + minAxisY * axis2[1] + minAxisZ * axis2[2];

    let vertex = getVertexFromAxisSigns(sign0, sign1, sign2);

    const collision = Collision.cubeFaceCubePoint(collider2, collider1, vertex, minAxisX, minAxisY, minAxisZ, collider1.vertices[vertex], directionX, directionY, directionZ);
    collisions.push(collision);
  } else {
    // Collider1 edge, collider2 edge
    const edgeAxisIndex = minimumSeparatingAxis.axisIndex - 6;

    const axis0 = collider1.axes[0];
    const axis1 = collider1.axes[1];
    const axis2 = collider1.axes[2];

    let minAxisX = minimumSeparatingAxis.axis[0] * minimumSeparatingAxis.axisSign;
    let minAxisY = minimumSeparatingAxis.axis[1] * minimumSeparatingAxis.axisSign;
    let minAxisZ = minimumSeparatingAxis.axis[2] * minimumSeparatingAxis.axisSign;

    let axis = Math.floor(edgeAxisIndex / 3);
    let sign0 = 0, sign1 = 0, sign2 = 0;
    if(axis != 0) sign0 = minAxisX * axis0[0] + minAxisY * axis0[1] + minAxisZ * axis0[2];
    if(axis != 1) sign1 = minAxisX * axis1[0] + minAxisY * axis1[1] + minAxisZ * axis1[2];
    if(axis != 2) sign2 = minAxisX * axis2[0] + minAxisY * axis2[1] + minAxisZ * axis2[2];

    let { vertex, nextVertex } = getEdgeFromSigns(sign0, sign1, sign2);

    const axis0B = collider2.axes[0];
    const axis1B = collider2.axes[1];
    const axis2B = collider2.axes[2];
    
    let sign0B = 0, sign1B = 0, sign2B = 0;
    if(axis != 0) sign0B = minAxisX * axis0B[0] + minAxisY * axis0B[1] + minAxisZ * axis0B[2];
    if(axis != 1) sign1B = minAxisX * axis1B[0] + minAxisY * axis1B[1] + minAxisZ * axis1B[2];
    if(axis != 2) sign2B = minAxisX * axis2B[0] + minAxisY * axis2B[1] + minAxisZ * axis2B[2];
    
    let { vertex: vertexB, nextVertex: nextVertexB } = getEdgeFromSigns(sign0B, sign1B, sign2B);

    const collision = Collision.cubeEdgeCubeEdge(
      collider1, collider2,
      vertex, vertexB, nextVertex, nextVertexB,
      minAxisX, minAxisY, minAxisZ,
      // getTwoClosestPointsOnEdges(vertex1.x, vertex1.y, vertex1.z, nvert1.x - vertex1.x, nvert1.y - vertex1.y, nvert1.z - vertex1.z, vertex2.x, vertex2.y, vertex2.z, nvert2.x - vertex2.x, nvert2.y - vertex2.y, nvert2.z - vertex2.z)
      // ??? idk
      // collider1.vertices[vertex][0], collider1.vertices[vertex][1], collider1.vertices[vertex][2]
    );
  }
}

function simulate(dt: number) {
  // TODO: Actually use dt lol
  for(let cube of cubes) {
    if(!cube.sleeping) cube.physicsTick();
  }

  updateActiveCollisions();

  // Get new collisions
  for(let a = 0; a < cubes.length; a++) {
    for(let b = a + 1; b < cubes.length; b++) {
      getCollisions(cubes[a], cubes[b]);
    }
  }

  // Resolve collisions
  // TODO
  // cullCollisions();
  // resolveCollisionVelocity();
  // resolveCollisionPenetration();

  // Finalize the update
  for(let cube of cubes) {
    cube.updateRendering();
    cube.checkSleep();
  }
}

for(let cube of cubes) {
  const geometry = new THREE.BoxGeometry(1, 1, 1);
  const material = new THREE.MeshNormalMaterial();
  const mesh = new THREE.Mesh(geometry, material);
  scene.add(mesh);
  cube.mesh = mesh;
}

const geometry = new THREE.PlaneGeometry(2, 2);
const material = new THREE.MeshBasicMaterial({ color: 0x222 });
const mesh = new THREE.Mesh(geometry, material);
mesh.rotation.x = -Math.PI / 2;
scene.add(mesh);

const camera = new THREE.PerspectiveCamera( 70, window.innerWidth / window.innerHeight, 0.01, 10 );
camera.position.z = 3;
camera.position.y = 1.5;
const renderer = new THREE.WebGLRenderer({ antialias: true });
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setAnimationLoop(animation);
document.body.appendChild(renderer.domElement);
const controls = new OrbitControls( camera, renderer.domElement );
let lastTime = 0;
function animation(time: number) {
  const dt = time - lastTime;
  lastTime = time;

  controls.update(time);

  renderer.render(scene, camera);
}

setInterval(() => {
  simulate(1000 / 60);
}, 1000 / 60);