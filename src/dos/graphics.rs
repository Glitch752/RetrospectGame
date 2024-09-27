use core::{arch::asm, intrinsics};

pub fn enter_graphics_mode() {
    unsafe {
        asm!("mov ax, 0x13", // ah = 0x00, al = 0x13
             "int 0x10");
    }
}

pub fn leave_graphics_mode() {
    unsafe {
        asm!("mov ax, 0x03", // ah = 0x00, al = 0x03
             "int 0x10");
    }
}

pub fn set_pixel(x: u16, y: u16, color: u8) {
    let page_number: i8 = 0;
    unsafe {
        asm!("mov ah, 0x0C", // ah = 0x0C, al = color, bh = page_number, cx = x, dx = y
             "int 0x10", in("al") color, in("bh") page_number, in("cx") x, in("dx") y);
    }
}

pub fn copy_framebuffer_to_screen(framebuffer: *const [u8], framebuffer_size: usize) {
    unsafe {
        // // We use movsd to move double words (4 bytes) from the framebuffer to the VRAM
        // asm!(
        //     "push esi",       // Save ESI on the stack
        //     "push edi",       // Save EDI on the stack

        //     // Set up the source in DS:SI
        //     "mov ds, ebx",    // Load the offset of the source into ESI
        //     "mov esi, 0",     // Clear ESI to point at 0x0000 (offset) within DS

        //     // Set up the destination in ES:DI (0xA000)
        //     "mov ax, 0xA000",  // Load the segment 0xA000 into AX
        //     "mov es, ax",      // Move it into ES (destination segment)
        //     "xor edi, edi",      // Clear DI to point at 0x0000 (offset) within 0xA000 segment

        //     // Perform the copy
        //     "rep movsd",       // Copy ECX double words from DS:ESI to ES:EDI

        //     "pop edi",        // Restore EDI from the stack
        //     "pop esi",        // Restore ESI from the stack
            
        //     in("ecx") framebuffer_size / 4,
        //     in("ebx") framebuffer as *const u8,
        //     out("ax") _,
        // );

        // Temporary: just copy it byte-by-byte
        let mut framebuffer_ptr = framebuffer as *const u8;
        for i in 0..framebuffer_size {
            let pixel = *framebuffer_ptr;
            // Set the memory at 0xA000:0x0000 + i to the pixel value
            asm!(
                "mov es, {seg}",      // Set the ES segment register
                "stosb",              // Store AL at ES:[DI] and increment DI
                seg = in(reg) 0xA000,   // Input the segment address
                in("di") i,      // Input the offset (0x0000 + i)
                in("al") pixel,   // Input the value to write
                options(nostack),        // No need for stack adjustment
            );

            framebuffer_ptr = framebuffer_ptr.add(1);
        }
    }
}

pub fn clear_screen() {
    unsafe {
        asm!("mov ax, 0x0", // ah = 0x00, al = 0x00
             "int 0x10");
    }
}

pub fn wait_for_vsync() {
    unsafe {
        asm!("mov dx, 0x3DA",
             "2:", // We need to use local labels in Rust assembly macro assembly blocks
             "in al, dx",
             "test al, 0x08",
             "jnz 2b");
    }
}