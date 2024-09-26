use core::intrinsics;

const fn num_bits<T>() -> usize { core::mem::size_of::<T>() * 8 }

pub const fn log_2(x: usize) -> usize {
    num_bits::<usize>() as usize - x.leading_zeros() as usize - 1
}

pub fn sin(theta: f32) -> f32 {
    unsafe { intrinsics::sinf32(theta) }
}

pub fn cos(theta: f32) -> f32 {
    unsafe { intrinsics::cosf32(theta) }
}