import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';

const scene = new THREE.Scene();

class Cube {
  // We're intentionally not using a lot of classes and three.js concepts here to make it easy to port to C
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

  t00: number = 0;
  t01: number = 0;
  t02: number = 0;
  t10: number = 0;
  t11: number = 0;
  t12: number = 0;
  t20: number = 0;
  t21: number = 0;
  t22: number = 0;

  constructor(size: number, x: number, y: number, z: number) {
    this.size = size;
    this.x = x;
    this.y = y;
    this.z = z;
  }

  physicsTick() {

  }

  checkSleep() {
    if(this.recentMotion < 2500 && this.recentRotation < 2500) {
      this.sleep();
    }
  }

  sleep() {
    this.clearVelocity();
    this.clearRotation();
    this.sleeping = true;
  }
  wakeUp() {
    this.recentMotion = 10000;
    this.recentRotation = 10000;
    this.sleeping = false;
  }

  clearVelocity() {
    this.velocityX = 0;
    this.velocityY = 0;
    this.velocityZ = 0;
  }
  clearRotation() {
    this.rotationX = 0;
    this.rotationY = 0;
    this.rotationZ = 0;
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

    // TODO: quaternion stuff idk
  }

  mesh?: THREE.Mesh;
  tempMatrix3 = new THREE.Matrix3();
  tempVector3 = new THREE.Vector3();
  updateRendering() {
    if(!this.mesh) return;
    // TEMPORARY
    this.t00 = 0.7071067811865476;
    this.t01 = -0.7071067811865475;
    this.t02 = 0;
    this.t10 = 0.7071067811865475;
    this.t11 = 0.7071067811865476;
    this.t12 = 0;
    this.t20 = 0;
    this.t21 = 0;
    this.t22 = 1;
    this.mesh.matrixAutoUpdate = false;
    this.mesh.matrix.setFromMatrix3(this.tempMatrix3.set(this.t00, this.t01, this.t02, this.t10, this.t11, this.t12, this.t20, this.t21, this.t22));
    this.mesh.matrix.setPosition(this.tempVector3.set(this.x, this.y, this.z));
  }
}

const cubes: Cube[] = [];
for(let x = -2; x <= 2; x++) for(let y = -2; y <= 2; y++) cubes.push(new Cube(0.1, x * 0.2, 0.05, y * 0.2));

function simulate(dt: number) {
  for(let cube of cubes) {
    if(!cube.sleeping) cube.physicsTick();
  }

  for(let a = 0; a < cubes.length; a++) {
    for(let b = a + 1; b < cubes.length; b++) {

    }
  }

  for(let cube of cubes) {
    cube.updateRendering();
    cube.checkSleep();
  }
}


for(let cube of cubes) {
  const geometry = new THREE.BoxGeometry(cube.size, cube.size, cube.size);
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
camera.position.z = 1;
camera.position.y = 0.5;
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

  simulate(dt);

  renderer.render(scene, camera);
}