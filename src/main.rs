#![no_std]
#![no_main]
#![feature(core_intrinsics)]

extern crate alloc;
use core::intrinsics;

use rust_dos::*;

entry!(main);

fn main() {
    println!("Hello, world!");
    
    // Donut.c ported to Rust
    let mut a: f32 = 0.0;
    let mut b: f32 = 0.0;
    
    loop {
        a += 0.07;
        b += 0.03;

        let sin_a = dos::math::sin(a);
        let cos_a = dos::math::cos(a);
        let sin_b = dos::math::sin(b);
        let cos_b = dos::math::cos(b);

        let mut z = [0.0; 1760];
        let mut buf: [char; 1760] = [' '; 1760];

        for j in (79..1760).step_by(80) {
            buf[j] = '\n';
        }

        let mut j: f32 = 0.0;
        while j < 6.28 {
            let j_sin = dos::math::sin(j);
            let j_cos = dos::math::cos(j);

            let mut i: f32 = 0.0;
            while i < 6.28 {
                let i_sin = dos::math::sin(i);
                let i_cos = dos::math::cos(i);

                let h = j_cos + 2.0;
                let d = 1.0 / (i_sin * h * sin_a + j_sin * cos_a + 5.0);
                let t = i_sin * h * cos_a - j_sin * sin_a;

                let x = unsafe { intrinsics::floorf32(40.0 + 30.0 * d * (i_cos * h * cos_b - t * sin_b)) };
                let y = unsafe { intrinsics::floorf32(12.0 + 15.0 * d * (i_cos * h * sin_b + t * cos_b)) };
                let o = x + 80.0 * y;
                
                if y >= 0.0 && y < 22.0 && x >= 0.0 && x < 79.0 {
                    if d > z[o as usize] {
                        z[o as usize] = d;
                        const CHARS: [char; 12] = ['.', ',', '-', '~', ':', ';', '=', '!', '*', '#', '$', '@'];
                        buf[o as usize] = CHARS[unsafe { intrinsics::floorf32(
                            intrinsics::maxnumf32(0.0, 8.0 * ((j_sin * sin_a - i_sin * j_cos * cos_a) * cos_b - i_sin * j_cos * sin_a - j_sin * cos_a - i_cos * j_cos * sin_b))
                        ) } as usize];
                    }
                }

                i += 0.02;
            }
            j += 0.07;
        }

        for k in 0..1760 {
            if buf[k] == '\n' {
                print!("\n\r");
            } else {
                print!("{}", buf[k]);
            }
        }

        a += 0.04;
        b += 0.02;
    }
}
