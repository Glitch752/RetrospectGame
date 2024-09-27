#![no_std]
#![no_main]
// #![feature(core_intrinsics)]

extern crate alloc;

use core::ops::{Add, Sub};

use dos::graphics::{copy_framebuffer_to_screen, enter_graphics_mode, wait_for_vsync};
use rust_dos::*;

entry!(main);

struct Vec3 {
    x: f32,
    y: f32,
    z: f32,
}

impl Vec3 {
    fn new(x: f32, y: f32, z: f32) -> Self {
        Self { x, y, z }
    }

    fn dot(&self, other: &Self) -> f32 {
        self.x * other.x + self.y * other.y + self.z * other.z
    }
}

impl Add for Vec3 {
    type Output = Self;
    fn add(self, other: Self) -> Self {
        Self {
            x: self.x + other.x,
            y: self.y + other.y,
            z: self.z + other.z,
        }
    }
}

impl Sub for Vec3 {
    type Output = Self;
    fn sub(self, other: Self) -> Self {
        Self {
            x: self.x - other.x,
            y: self.y - other.y,
            z: self.z - other.z,
        }
    }
}

struct Cube {
    position: Vec3,
    rotation: Vec3,
    scale: Vec3,
    color: Vec3,
}

// Mode 13h is 320x200 with 256 colors
const SCREEN_WIDTH: usize = 320;
const SCREEN_HEIGHT: usize = 200;
const FOCAL_LENGTH: f32 = 1.0;
const FOV: f32 = 1.0;

// static mut Z: [i8; SCREEN_WIDTH * SCREEN_HEIGHT] = [i8::MAX; SCREEN_WIDTH * SCREEN_HEIGHT];
static mut FRAMEBUFFER: [u8; SCREEN_WIDTH * SCREEN_HEIGHT] = [0; SCREEN_WIDTH * SCREEN_HEIGHT]; // 1 byte per pixel

// fn display_point(point: Vec3) {
//     // Transform into screen space
//     let x = (point.x / point.z) * FOCAL_LENGTH + (SCREEN_WIDTH as f32 / 2.0);
//     let y = (point.y / point.z) * FOCAL_LENGTH + (SCREEN_HEIGHT as f32 / 2.0);

//     // Clamp to screen bounds
//     if x < 0.0 || x >= SCREEN_WIDTH as f32 || y < 0.0 || y >= SCREEN_HEIGHT as f32 {
//         return;
//     }

//     let x = x as usize;
//     let y = y as usize;

//     let z = point.z as i8;

//     if z > unsafe { Z[y * SCREEN_WIDTH + x] } {
//         return;
//     }

//     unsafe {
//         Z[y * SCREEN_WIDTH + x] = z;
//         FRAMEBUFFER[y * SCREEN_WIDTH + x] = 255;
//     }
// }

fn set_pixel_fb(x: usize, y: usize, color: u8) {
    unsafe {
        FRAMEBUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

fn main() {
    enter_graphics_mode();
    for x in 0..SCREEN_WIDTH {
        for y in 0..SCREEN_HEIGHT {
            let color = (x % 16) as u8 | ((y % 16) as u8) << 4;
            set_pixel_fb(x, y, color);
        }
    }
    copy_framebuffer_to_screen(unsafe { core::ptr::addr_of!(FRAMEBUFFER) }, SCREEN_WIDTH * SCREEN_HEIGHT);
    // loop {
    //     for i in 0..SCREEN_WIDTH * SCREEN_HEIGHT {
    //         unsafe {
    //             Z[i] = i8::MAX;
    //             BUF[i] = ' ';
    //         }
    //     }

    //     display_point(Vec3::new(0.0, 0.0, 1.0));
    //     display_point(Vec3::new(1.0, 0.0, 1.0));
    //     display_point(Vec3::new(0.0, 1.0, 1.0));
        
    //     for i in 0..SCREEN_WIDTH {
    //         display_point(Vec3::new(i as f32, 0.0, 1.0));
    //         display_point(Vec3::new(i as f32, SCREEN_HEIGHT as f32 - 1.0, 1.0));
    //     }

    //     print!("\x1B[H");
    //     for k in 0..SCREEN_WIDTH * SCREEN_HEIGHT {
    //         if k % SCREEN_WIDTH == 0 {
    //             print!("\n\r");
    //         } else {
    //             print!("{}", unsafe { BUF[k] });
    //         }
    //     }
    // }
}
