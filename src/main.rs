#![no_std]
#![no_main]
// #![feature(core_intrinsics)]

extern crate alloc;

use rust_dos::*;

entry!(main);

fn rotate(mul: i32, shift: i32, x: &mut i32, y: &mut i32) {
    let mut old_x = *x;
    *x -= mul * *y >> shift;
    *y += mul * old_x >> shift;
    old_x = 3145728 - *x * *x - *y * *y >> 11;
    *x = *x * old_x >> 10;
    *y = *y * old_x >> 10;
}

fn main() {
    let mut sin_a: i32 = 1024;
    let mut cos_a: i32 = 0;
    let mut sin_b: i32 = 1024;
    let mut cos_b: i32 = 0;
    
    loop {
        let mut z = [i8::MAX; 1760];
        let mut buf: [char; 1760] = [' '; 1760];

        let mut sj = 0;
        let mut cj = 1024;
        for _ in 0..90 {
            let mut si = 0;
            let mut ci = 1024;

            for _ in 0..324 {
                let r1 = 1;
                let r2 = 2048;
                let k2 = 5120 * 1024;
        
                let x0 = r1 * cj + r2;
                let x1 = ci * x0 >> 10;
                let x2 = cos_a * sj >> 10;
                let x3 = si * x0 >> 10;
                let x4 = r1 * x2 - (sin_a * x3 >> 10);
                let x5 = sin_a * sj >> 10;
                let x6 = k2 + r1 * 1024 * x5 + cos_a * x3;
                let x7 = cj * si >> 10;
                let x = 40 + 30 * (cos_b * x1 - sin_b * x4) / x6;
                let y = 12 + 15 * (cos_b * x4 + sin_b * x1) / x6;
                let n = (-cos_a * x7 - cos_b * ((-sin_a * x7 >> 10) + x2) - ci * (cj * sin_b >> 10) >> 10) - x5 >> 7;

                let o = (x + 80 * y) as usize;
                let zz: i8 = ((x6 - k2) >> 15) as i8;
                if y < 22 && y > 0 && x > 0 && x < 80 && zz < z[o] {
                    z[o] = zz;
                    buf[o] = ['.', ',', '-', '~', ':', ';', '=', '!', '*', '#', '$', '@'][(if n > 0 { n } else { 0 }) as usize];
                }
                rotate(5, 8, &mut ci, &mut si);
            }
            rotate(9, 7, &mut cj, &mut sj);
        }
 
        print!("\x1B[H");
        for k in 0..1760 {
            if k % 80 == 0 {
                print!("\n\r");
            } else {
                print!("{}", buf[k]);
            }
        }

        rotate(5, 7, &mut cos_a, &mut sin_a);
        rotate(5, 8, &mut cos_b, &mut sin_b);
    }
}
